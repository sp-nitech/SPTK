// ------------------------------------------------------------------------ //
// Copyright 2021 SPTK Working Group                                        //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ------------------------------------------------------------------------ //

#include <algorithm>  // std::transform
#include <cmath>      // std::log
#include <fstream>    // std::ifstream
#include <iomanip>    // std::setw
#include <iostream>   // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>    // std::ostringstream
#include <vector>     // std::vector

#include "Getopt/getoptwin.h"
#include "SPTK/conversion/generalized_cepstrum_gain_normalization.h"
#include "SPTK/conversion/mel_cepstrum_to_mlsa_digital_filter_coefficients.h"
#include "SPTK/filter/mglsa_digital_filter.h"
#include "SPTK/input/input_source_from_stream.h"
#include "SPTK/input/input_source_interpolation.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultNumFilterOrder(25);
const double kDefaultAlpha(0.35);
const int kDefaultNumStage(0);
const int kDefaultFramePeriod(100);
const int kDefaultInterpolationPeriod(1);
const int kDefaultNumPadeOrder(4);
const bool kDefaultTranspositionFlag(false);
const bool kDefaultGainFlag(true);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " mglsadf - MGLSA digital filter for speech synthesis" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       mglsadf [ options ] mgcfile [ infile ] > stdout" << std::endl;  // NOLINT
  *stream << "  options:" << std::endl;
  *stream << "       -m m  : order of filter coefficients  (   int)[" << std::setw(5) << std::right << kDefaultNumFilterOrder      << "][    0 <= m <=     ]" << std::endl;  // NOLINT
  *stream << "       -a a  : all-pass constant             (double)[" << std::setw(5) << std::right << kDefaultAlpha               << "][ -1.0 <  a <  1.0 ]" << std::endl;  // NOLINT
  *stream << "       -c c  : gamma = -1 / c                (   int)[" << std::setw(5) << std::right << kDefaultNumStage            << "][    0 <= c <=     ]" << std::endl;  // NOLINT
  *stream << "       -p p  : frame period                  (   int)[" << std::setw(5) << std::right << kDefaultFramePeriod         << "][    1 <= p <=     ]" << std::endl;  // NOLINT
  *stream << "       -i i  : interpolation period          (   int)[" << std::setw(5) << std::right << kDefaultInterpolationPeriod << "][    0 <= i <= p/2 ]" << std::endl;  // NOLINT
  *stream << "       -P P  : order of Pade approximation   (   int)[" << std::setw(5) << std::right << kDefaultNumPadeOrder        << "][    4 <= P <= 7   ]" << std::endl;  // NOLINT
  *stream << "       -t    : transpose filter              (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultTranspositionFlag) << "]" << std::endl;  // NOLINT
  *stream << "       -k    : filtering without gain        (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(!kDefaultGainFlag)         << "]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  mgcfile:" << std::endl;
  *stream << "       mel-generalized cepstral coefficients (double)" << std::endl;  // NOLINT
  *stream << "  infile:" << std::endl;
  *stream << "       filter input                          (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       filter output                         (double)" << std::endl;  // NOLINT
  *stream << "  notice:" << std::endl;
  *stream << "       if i = 0, don't interpolate filter coefficients" << std::endl;  // NOLINT
  *stream << "       if c = 0, MLSA filter is used" << std::endl;
  *stream << "       if c > 0, MGLSA filter is used and P is ignored" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

class InputSourcePreprocessingForMelCepstrum
    : public sptk::InputSourceInterface {
 public:
  InputSourcePreprocessingForMelCepstrum(double alpha, double gamma,
                                         bool gain_flag,
                                         sptk::InputSourceInterface* source)
      : gamma_(gamma),
        gain_flag_(gain_flag),
        source_(source),
        mel_cepstrum_to_mlsa_digital_filter_coefficients_(
            source ? source->GetSize() - 1 : 0, alpha),
        generalized_cepstrum_gain_normalization_(
            source ? source->GetSize() - 1 : 0, gamma),
        is_valid_(true) {
    if (NULL == source || !source->IsValid() ||
        !mel_cepstrum_to_mlsa_digital_filter_coefficients_.IsValid() ||
        !generalized_cepstrum_gain_normalization_.IsValid()) {
      is_valid_ = false;
      return;
    }
  }

  ~InputSourcePreprocessingForMelCepstrum() {
  }

  bool GetGainFlag() const {
    return gain_flag_;
  }

  virtual int GetSize() const {
    return source_ ? source_->GetSize() : 0;
  }

  virtual bool IsValid() const {
    return is_valid_;
  }

  virtual bool Get(std::vector<double>* mlsa_digital_filter_coefficients) {
    if (!is_valid_) {
      return false;
    }

    if (!source_->Get(&mel_cepstrum_)) {
      return false;
    }

    if (!mel_cepstrum_to_mlsa_digital_filter_coefficients_.Run(
            mel_cepstrum_, mlsa_digital_filter_coefficients)) {
      return false;
    }

    if (0.0 != gamma_) {
      if (!generalized_cepstrum_gain_normalization_.Run(
              mlsa_digital_filter_coefficients)) {
        return false;
      }
      if (gain_flag_) {
        (*mlsa_digital_filter_coefficients)[0] =
            std::log((*mlsa_digital_filter_coefficients)[0]);
      }
      std::transform(mlsa_digital_filter_coefficients->begin() + 1,
                     mlsa_digital_filter_coefficients->end(),
                     mlsa_digital_filter_coefficients->begin() + 1,
                     [this](double b) { return b * gamma_; });
    }

    if (!gain_flag_) {
      (*mlsa_digital_filter_coefficients)[0] = 0.0;  // exp(0) = 1
    }

    return true;
  }

 private:
  const double gamma_;
  const bool gain_flag_;

  InputSourceInterface* source_;

  const sptk::MelCepstrumToMlsaDigitalFilterCoefficients
      mel_cepstrum_to_mlsa_digital_filter_coefficients_;
  const sptk::GeneralizedCepstrumGainNormalization
      generalized_cepstrum_gain_normalization_;

  bool is_valid_;

  std::vector<double> mel_cepstrum_;

  DISALLOW_COPY_AND_ASSIGN(InputSourcePreprocessingForMelCepstrum);
};

}  // namespace

/**
 * @a mglsadf [ @e option ] @e mgcfile [ @e infile ]
 *
 * - @b -m @e int
 *   - order of coefficients @f$(0 \le M)@f$
 * - @b -a @e double
 *   - all-pass constant @f$(|\alpha| < 1)@f$
 * - @b -c @e int
 *   - gamma @f$\gamma = -1 / C@f$ @f$(1 \le C)@f$
 * - @b -p @e int
 *   - frame period @f$(1 \le P)@f$
 * - @b -i @e int
 *   - interpolation period @f$(0 \le I \le P/2)@f$
 * - @b -P @e int
 *   - order of Pade approximation @f$(4 \le L \le 7)@f$
 * - @b -t @e bool
 *   - transpose filter
 * - @b -k @e bool
 *   - filtering without gain
 * - @b mgcfile @e str
 *   - double-type mel-generalized cepstral coefficients
 * - @b infile @e str
 *   - double-type input sequence
 * - @b stdout
 *   - double-type output sequence
 *
 * In the below example, an exciation signal generated from pitch information is
 * passed through the MLSA filter built from mel-cepstral coefficients
 * @c data.mcep.
 *
 * @code{.sh}
 *   excite < data.pitch | mglsadf data.mcep > data.syn
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_filter_order(kDefaultNumFilterOrder);
  double alpha(kDefaultAlpha);
  int num_stage(kDefaultNumStage);
  int frame_period(kDefaultFramePeriod);
  int interpolation_period(kDefaultInterpolationPeriod);
  int num_pade_order(kDefaultNumPadeOrder);
  bool transposition_flag(kDefaultTranspositionFlag);
  bool gain_flag(kDefaultGainFlag);

  for (;;) {
    const int option_char(
        getopt_long(argc, argv, "m:a:c:p:i:P:tkh", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_filter_order) ||
            num_filter_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("mglsadf", error_message);
          return 1;
        }
        break;
      }
      case 'a': {
        if (!sptk::ConvertStringToDouble(optarg, &alpha) ||
            !sptk::IsValidAlpha(alpha)) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -a option must be in (-1.0, 1.0)";
          sptk::PrintErrorMessage("mglsadf", error_message);
          return 1;
        }
        break;
      }
      case 'c': {
        if (!sptk::ConvertStringToInteger(optarg, &num_stage) ||
            num_stage < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -c option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("mglsadf", error_message);
          return 1;
        }
        break;
      }
      case 'p': {
        if (!sptk::ConvertStringToInteger(optarg, &frame_period) ||
            frame_period <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -p option must be a positive integer";
          sptk::PrintErrorMessage("mglsadf", error_message);
          return 1;
        }
        break;
      }
      case 'i': {
        if (!sptk::ConvertStringToInteger(optarg, &interpolation_period) ||
            interpolation_period < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -i option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("mglsadf", error_message);
          return 1;
        }
        break;
      }
      case 'P': {
        const int min(4);
        const int max(7);
        if (!sptk::ConvertStringToInteger(optarg, &num_pade_order) ||
            !sptk::IsInRange(num_pade_order, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -P option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("mglsadf", error_message);
          return 1;
        }
        break;
      }
      case 't': {
        transposition_flag = true;
        break;
      }
      case 'k': {
        gain_flag = false;
        break;
      }
      case 'h': {
        PrintUsage(&std::cout);
        return 0;
      }
      default: {
        PrintUsage(&std::cerr);
        return 1;
      }
    }
  }

  if (frame_period / 2 < interpolation_period) {
    std::ostringstream error_message;
    error_message << "Interpolation period must be equal to or less than half "
                  << "frame period";
    sptk::PrintErrorMessage("mglsadf", error_message);
    return 1;
  }

  // Get input file names.
  const char* filter_coefficients_file;
  const char* filter_input_file;
  const int num_input_files(argc - optind);
  if (2 == num_input_files) {
    filter_coefficients_file = argv[argc - 2];
    filter_input_file = argv[argc - 1];
  } else if (1 == num_input_files) {
    filter_coefficients_file = argv[argc - 1];
    filter_input_file = NULL;
  } else {
    std::ostringstream error_message;
    error_message << "Just two input files, mgcfile and infile, are required";
    sptk::PrintErrorMessage("mglsadf", error_message);
    return 1;
  }

  // Open stream for reading filter coefficients.
  std::ifstream ifs1;
  ifs1.open(filter_coefficients_file, std::ios::in | std::ios::binary);
  if (ifs1.fail()) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << filter_coefficients_file;
    sptk::PrintErrorMessage("mglsadf", error_message);
    return 1;
  }
  std::istream& stream_for_filter_coefficients(ifs1);

  // Open stream for reading input signals.
  std::ifstream ifs2;
  ifs2.open(filter_input_file, std::ios::in | std::ios::binary);
  if (ifs2.fail() && NULL != filter_input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << filter_input_file;
    sptk::PrintErrorMessage("mglsadf", error_message);
    return 1;
  }
  std::istream& stream_for_filter_input(ifs2.fail() ? std::cin : ifs2);

  // Prepare variables for filtering.
  const int filter_length(num_filter_order + 1);
  std::vector<double> filter_coefficients(filter_length);
  sptk::InputSourceFromStream input_source(false, filter_length,
                                           &stream_for_filter_coefficients);
  const double gamma((0 == num_stage) ? 0.0 : -1.0 / num_stage);
  InputSourcePreprocessingForMelCepstrum preprocessing(alpha, gamma, gain_flag,
                                                       &input_source);
  sptk::InputSourceInterpolation interpolation(
      frame_period, interpolation_period, true, &preprocessing);
  if (!interpolation.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize InputSource";
    sptk::PrintErrorMessage("mglsadf", error_message);
    return 1;
  }

  sptk::MglsaDigitalFilter filter(num_filter_order, num_pade_order, num_stage,
                                  alpha, transposition_flag);
  sptk::MglsaDigitalFilter::Buffer buffer;
  if (!filter.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize MglsaDigitalFilter";
    sptk::PrintErrorMessage("mglsadf", error_message);
    return 1;
  }

  double signal;

  while (sptk::ReadStream(&signal, &stream_for_filter_input)) {
    if (!interpolation.Get(&filter_coefficients)) {
      std::ostringstream error_message;
      error_message << "Cannot get filter coefficients";
      sptk::PrintErrorMessage("mglsadf", error_message);
      return 1;
    }

    if (!filter.Run(filter_coefficients, &signal, &buffer)) {
      std::ostringstream error_message;
      error_message << "Failed to apply MGLSA digital filter";
      sptk::PrintErrorMessage("mglsadf", error_message);
      return 1;
    }

    if (!sptk::WriteStream(signal, &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write a filter output";
      sptk::PrintErrorMessage("mglsadf", error_message);
      return 1;
    }
  }

  return 0;
}
