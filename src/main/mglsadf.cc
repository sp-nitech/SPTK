// ----------------------------------------------------------------- //
//             The Speech Signal Processing Toolkit (SPTK)           //
//             developed by SPTK Working Group                       //
//             http://sp-tk.sourceforge.net/                         //
// ----------------------------------------------------------------- //
//                                                                   //
//  Copyright (c) 1984-2007  Tokyo Institute of Technology           //
//                           Interdisciplinary Graduate School of    //
//                           Science and Engineering                 //
//                                                                   //
//                1996-2019  Nagoya Institute of Technology          //
//                           Department of Computer Science          //
//                                                                   //
// All rights reserved.                                              //
//                                                                   //
// Redistribution and use in source and binary forms, with or        //
// without modification, are permitted provided that the following   //
// conditions are met:                                               //
//                                                                   //
// - Redistributions of source code must retain the above copyright  //
//   notice, this list of conditions and the following disclaimer.   //
// - Redistributions in binary form must reproduce the above         //
//   copyright notice, this list of conditions and the following     //
//   disclaimer in the documentation and/or other materials provided //
//   with the distribution.                                          //
// - Neither the name of the SPTK working group nor the names of its //
//   contributors may be used to endorse or promote products derived //
//   from this software without specific prior written permission.   //
//                                                                   //
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            //
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       //
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          //
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          //
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS //
// BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          //
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   //
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     //
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON //
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   //
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    //
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           //
// POSSIBILITY OF SUCH DAMAGE.                                       //
// ----------------------------------------------------------------- //

#include <getopt.h>    // getopt_long
#include <algorithm>   // std::transform
#include <cmath>       // std::log
#include <fstream>     // std::ifstream
#include <functional>  // std::bind1st, std::multiplies
#include <iomanip>     // std::setw
#include <iostream>    // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>     // std::ostringstream
#include <vector>      // std::vector

#include "SPTK/converter/mel_cepstrum_to_mlsa_digital_filter_coefficients.h"
#include "SPTK/filter/mglsa_digital_filter.h"
#include "SPTK/input/input_source_from_stream.h"
#include "SPTK/input/input_source_interpolation.h"
#include "SPTK/normalizer/generalized_cepstrum_gain_normalization.h"
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
  *stream << "       -p p  : frame period                  (   int)[" << std::setw(5) << std::right << kDefaultFramePeriod         << "][    0 <  p <=     ]" << std::endl;  // NOLINT
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
  *stream << "       otherwise MGLSA filter is used and P is ignored" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

class InputSourcePreprocessingForMelCepstrum
    : public sptk::InputSourceInterface {
 public:
  //
  InputSourcePreprocessingForMelCepstrum(double alpha, double gamma,
                                         bool gain_flag,
                                         sptk::InputSourceInterface* source)
      : gain_flag_(gain_flag),
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
    }
  }

  //
  ~InputSourcePreprocessingForMelCepstrum() {
  }

  //
  double GetAlpha() const {
    return mel_cepstrum_to_mlsa_digital_filter_coefficients_.GetAlpha();
  }

  //
  double GetGamma() const {
    return generalized_cepstrum_gain_normalization_.GetGamma();
  }

  //
  bool GetGainFlag() const {
    return gain_flag_;
  }

  //
  virtual int GetSize() const {
    return source_ ? source_->GetSize() : 0;
  }

  //
  virtual bool IsValid() const {
    return is_valid_;
  }

  //
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

    // MLSA filter
    if (0.0 == GetGamma()) {
      if (!gain_flag_) (*mlsa_digital_filter_coefficients)[0] = 0.0;
      return true;
    }

    // MGLSA filter
    if (!generalized_cepstrum_gain_normalization_.Run(
            *mlsa_digital_filter_coefficients,
            &temporary_mlsa_digital_filter_coefficients_)) {
      return false;
    }
    std::transform(temporary_mlsa_digital_filter_coefficients_.begin() + 1,
                   temporary_mlsa_digital_filter_coefficients_.end(),
                   mlsa_digital_filter_coefficients->begin() + 1,
                   std::bind1st(std::multiplies<double>(), GetGamma()));
    if (gain_flag_) {
      if (temporary_mlsa_digital_filter_coefficients_[0] <= 0.0) return false;
      (*mlsa_digital_filter_coefficients)[0] =
          std::log(temporary_mlsa_digital_filter_coefficients_[0]);
    } else {
      (*mlsa_digital_filter_coefficients)[0] = 0.0;
    }
    return true;
  }

 private:
  //
  const bool gain_flag_;

  //
  InputSourceInterface* source_;

  //
  const sptk::MelCepstrumToMlsaDigitalFilterCoefficients
      mel_cepstrum_to_mlsa_digital_filter_coefficients_;

  //
  const sptk::GeneralizedCepstrumGainNormalization
      generalized_cepstrum_gain_normalization_;

  //
  std::vector<double> mel_cepstrum_;

  //
  std::vector<double> temporary_mlsa_digital_filter_coefficients_;

  //
  bool is_valid_;

  //
  DISALLOW_COPY_AND_ASSIGN(InputSourcePreprocessingForMelCepstrum);
};

}  // namespace

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
  double filter_input, filter_output;

  sptk::MglsaDigitalFilter filter(num_filter_order, num_pade_order, num_stage,
                                  alpha, transposition_flag);
  sptk::MglsaDigitalFilter::Buffer buffer;

  if (!interpolation.IsValid() || !filter.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to set condition for filtering";
    sptk::PrintErrorMessage("mglsadf", error_message);
    return 1;
  }

  while (sptk::ReadStream(&filter_input, &stream_for_filter_input)) {
    if (!interpolation.Get(&filter_coefficients)) {
      std::ostringstream error_message;
      error_message << "Cannot get filter coefficients";
      sptk::PrintErrorMessage("mglsadf", error_message);
      return 1;
    }

    if (!filter.Run(filter_coefficients, filter_input, &filter_output,
                    &buffer)) {
      std::ostringstream error_message;
      error_message << "Failed to apply MGLSA digital filter";
      sptk::PrintErrorMessage("mglsadf", error_message);
      return 1;
    }

    if (!sptk::WriteStream(filter_output, &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write a filter output";
      sptk::PrintErrorMessage("mglsadf", error_message);
      return 1;
    }
  }

  return 0;
}
