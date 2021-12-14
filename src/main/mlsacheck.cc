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

#include <fstream>   // std::ifstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream
#include <vector>    // std::vector

#include "Getopt/getoptwin.h"
#include "SPTK/check/mlsa_digital_filter_stability_check.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum WarningType { kIgnore = 0, kWarn, kExit, kNumWarningTypes };

enum StabilityCondition {
  kKeepingLogApproximationError,
  kKeepingFilterStability,
  kNumConditions,
};

const int kDefaultNumFilterOrder(25);
const int kDefaultFftLength(256);
const double kDefaultAlpha(0.35);
const int kDefaultNumPadeOrder(4);
const WarningType kDefaultWarningType(kWarn);
const StabilityCondition kDefaultStabilityCondition(
    kKeepingLogApproximationError);
const sptk::MlsaDigitalFilterStabilityCheck::ModificationType
    kDefaultModificationType(
        sptk::MlsaDigitalFilterStabilityCheck::ModificationType::kClipping);
const bool kDefaultFastModeFlag(false);
const bool kDefaultModificationFlag(false);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " mlsacheck - check stability of MLSA digital filter" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       mlsacheck [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -m m  : order of filter coefficients      (   int)[" << std::setw(5) << std::right << kDefaultNumFilterOrder     << "][    0 <= m <=     ]" << std::endl;  // NOLINT
  *stream << "       -l l  : FFT length                        (   int)[" << std::setw(5) << std::right << kDefaultFftLength          << "][    m <  l <=     ]" << std::endl;  // NOLINT
  *stream << "       -a a  : all-pass constant                 (double)[" << std::setw(5) << std::right << kDefaultAlpha              << "][ -1.0 <  a <  1.0 ]" << std::endl;  // NOLINT
  *stream << "       -P P  : order of Pade approximation       (   int)[" << std::setw(5) << std::right << kDefaultNumPadeOrder       << "][    4 <= P <= 7   ]" << std::endl;  // NOLINT
  *stream << "       -e e  : warning type of unstable index    (   int)[" << std::setw(5) << std::right << kDefaultWarningType        << "][    0 <= e <= 2   ]" << std::endl;  // NOLINT
  *stream << "                 0 (no warning)" << std::endl;
  *stream << "                 1 (output the index to stderr)" << std::endl;
  *stream << "                 2 (output the index to stderr" << std::endl;
  *stream << "                    and exit immediately)" << std::endl;
  *stream << "       -R R  : threshold value                   (double)[" << std::setw(5) << std::right << "N/A"                      << "][  0.0 <  R <=     ]" << std::endl;  // NOLINT
  *stream << "       -r    : stability condition               (   int)[" << std::setw(5) << std::right << kDefaultStabilityCondition << "][    0 <= r <= 1   ]" << std::endl;  // NOLINT
  *stream << "                 0 (keeping maximum log approximation error)" << std::endl;  // NOLINT
  *stream << "                 1 (keeping filter stability)" << std::endl;
  *stream << "       -t    : modification type                 (   int)[" << std::setw(5) << std::right << kDefaultModificationType   << "][    0 <= t <= 1   ]" << std::endl;  // NOLINT
  *stream << "                 0 (clipping)" << std::endl;
  *stream << "                 1 (scaling)" << std::endl;
  *stream << "       -f    : fast mode                         (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultFastModeFlag)                          << "]" << std::endl;  // NOLINT
  *stream << "       -x    : perform modification              (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultModificationFlag)                      << "]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       mel-cepstrum                              (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       modified mel-cepstrum                     (double)" << std::endl;  // NOLINT
  *stream << "  notice:" << std::endl;
  *stream << "       value of l must be a power of 2" << std::endl;
  *stream << "       if -R option is not specified, an appropriate threshold is determined by -r and -P options" << std::endl;  // NOLINT
  *stream << "       -t option is valid only if -f option is not specified" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a mlsacheck [ @e option ] [ @e infile ]
 *
 * - @b -m @e int
 *   - order of mel-cepstrum @f$(0 \le M)@f$
 * - @b -l @e int
 *   - FFT length @f$(M < L)@f$
 * - @b -a @e double
 *   - all-pass constant @f$(|\alpha| < 1)@f$
 * - @b -P @e int
 *   - order of Pade approximation @f$(4 \le P \le 7)@f$
 * - @b -e @e int
 *   - warning type
 *     \arg @c 0 no warning
 *     \arg @c 1 output index
 *     \arg @c 2 output index and exit immediately
 * - @b -R @e double
 *   - threshold value @f$(0 < R)@f$
 * - @b -r @e int
 *   - stability condition
 *     \arg @c 0 keep maximum log approximation error
 *     \arg @c 1 keep filter stability
 * - @b -t @e int
 *   - modification type
 *     \arg @c 0 clipping
 *     \arg @c 1 scaling
 * - @b -f @e bool
 *   - fast mode
 * - @b -x @e bool
 *   - perform modification
 * - @b infile @e str
 *   - double-type mel-cepstrum
 * - @b stdout
 *   - double-type modified mel-cepstrum
 *
 * If @c -R option is not specified, the threshold value is automatically
 * determined according to the below table.
 *
 * @rst
 * +---+---------+---------+------------+
 * | P | R (r=0) | R (r=1) | E_max [dB] |
 * +===+=========+=========+============+
 * | 4 | 4.5     |  6.20   | 0.24       |
 * +---+---------+---------+------------+
 * | 5 | 6.0     |  7.65   | 0.27       |
 * +---+---------+---------+------------+
 * | 6 | 7.4     |  9.13   | 0.25       |
 * +---+---------+---------+------------+
 * | 7 | 8.9     | 10.60   | 0.26       |
 * +---+---------+---------+------------+
 * @endrst
 *
 * In the following example, the stability of MLSA filter of 49-th order
 * mel-cepstral coefficients read from @c data.mcep are checked and modified:
 *
 * @code{.sh}
 *   mlsacheck -m 49 -a 0.55 -P 5 -l 4096 -r 1 -x data.mcep > data2.mcep
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_filter_order(kDefaultNumFilterOrder);
  int fft_length(kDefaultFftLength);
  double alpha(kDefaultAlpha);
  int num_pade_order(kDefaultNumPadeOrder);
  WarningType warning_type(kDefaultWarningType);
  double threshold(0.0);
  StabilityCondition stability_condition(kDefaultStabilityCondition);
  sptk::MlsaDigitalFilterStabilityCheck::ModificationType modification_type(
      kDefaultModificationType);
  bool fast_mode_flag(kDefaultFastModeFlag);
  bool modification_flag(kDefaultModificationFlag);

  for (;;) {
    const int option_char(
        getopt_long(argc, argv, "m:l:a:P:e:R:r:t:fxh", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_filter_order) ||
            num_filter_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("mlsacheck", error_message);
          return 1;
        }
        break;
      }
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &fft_length)) {
          std::ostringstream error_message;
          error_message << "The argument for the -l option must be an integer";
          sptk::PrintErrorMessage("mlsacheck", error_message);
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
          sptk::PrintErrorMessage("mlsacheck", error_message);
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
          sptk::PrintErrorMessage("mlsacheck", error_message);
          return 1;
        }
        break;
      }
      case 'e': {
        const int min(0);
        const int max(static_cast<int>(kNumWarningTypes) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -e option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("mlsacheck", error_message);
          return 1;
        }
        warning_type = static_cast<WarningType>(tmp);
        break;
      }
      case 'R': {
        if (!sptk::ConvertStringToDouble(optarg, &threshold) ||
            threshold <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -r option must be a positive number";
          sptk::PrintErrorMessage("mlsacheck", error_message);
          return 1;
        }
        break;
      }
      case 'r': {
        const int min(0);
        const int max(static_cast<int>(StabilityCondition::kNumConditions) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -r option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("mlsacheck", error_message);
          return 1;
        }
        stability_condition = static_cast<StabilityCondition>(tmp);
        break;
      }
      case 'f': {
        fast_mode_flag = true;
        break;
      }
      case 'x': {
        modification_flag = true;
        break;
      }
      case 't': {
        const int min(0);
        const int max(
            static_cast<int>(sptk::MlsaDigitalFilterStabilityCheck::
                                 ModificationType::kNumModificationTypes) -
            1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -t option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("mlsacheck", error_message);
          return 1;
        }
        modification_type = static_cast<
            sptk::MlsaDigitalFilterStabilityCheck::ModificationType>(tmp);
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

  if (!fast_mode_flag && fft_length <= num_filter_order) {
    std::ostringstream error_message;
    error_message
        << "Order of filter coefficients must be less than FFT length";
    sptk::PrintErrorMessage("mlsacheck", error_message);
    return 1;
  }

  if (0.0 == threshold) {
    const bool is_strict(kKeepingLogApproximationError == stability_condition);
    if (4 == num_pade_order) {
      threshold = is_strict ? 4.5 : 6.20;
    } else if (5 == num_pade_order) {
      threshold = is_strict ? 6.0 : 7.65;
    } else if (6 == num_pade_order) {
      threshold = is_strict ? 7.4 : 9.13;
    } else if (7 == num_pade_order) {
      threshold = is_strict ? 8.9 : 10.6;
    }
  }

  const int num_rest_args(argc - optind);
  if (1 < num_rest_args) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("mlsacheck", error_message);
    return 1;
  }
  const char* input_file(0 == num_rest_args ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("mlsacheck", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::MlsaDigitalFilterStabilityCheck* mlsa_digital_filter_stability_check;
  if (fast_mode_flag) {
    mlsa_digital_filter_stability_check =
        new sptk::MlsaDigitalFilterStabilityCheck(num_filter_order, alpha,
                                                  threshold);
  } else {
    mlsa_digital_filter_stability_check =
        new sptk::MlsaDigitalFilterStabilityCheck(
            num_filter_order, alpha, threshold, fft_length, modification_type);
  }
  sptk::MlsaDigitalFilterStabilityCheck::Buffer buffer;
  if (!mlsa_digital_filter_stability_check->IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize MlsaDigitalFilterStabilityCheck";
    sptk::PrintErrorMessage("mlsacheck", error_message);
    delete mlsa_digital_filter_stability_check;
    return 1;
  }

  int result(0);
  const int length(num_filter_order + 1);
  std::vector<double> mel_cepstrum(length);

  for (int frame_index(0); sptk::ReadStream(false, 0, 0, length, &mel_cepstrum,
                                            &input_stream, NULL);
       ++frame_index) {
    bool is_stable(false);
    double maximum_amplitude(0.0);
    if (modification_flag) {
      if (!mlsa_digital_filter_stability_check->Run(
              &mel_cepstrum, &is_stable, &maximum_amplitude, &buffer)) {
        std::ostringstream error_message;
        error_message << "Failed to check stability of MLSA digital filter";
        sptk::PrintErrorMessage("mlsacheck", error_message);
        result = 1;
        break;
      }
    } else {
      if (!mlsa_digital_filter_stability_check->Run(
              mel_cepstrum, NULL, &is_stable, &maximum_amplitude, &buffer)) {
        std::ostringstream error_message;
        error_message << "Failed to check stability of MLSA digital filter";
        sptk::PrintErrorMessage("mlsacheck", error_message);
        result = 1;
        break;
      }
    }

    if (!is_stable && kIgnore != warning_type) {
      std::ostringstream error_message;
      error_message << frame_index << "th frame is unstable ("
                    << "maximum = " << maximum_amplitude << ", "
                    << "threshold = " << threshold << ")";
      sptk::PrintErrorMessage("mlsacheck", error_message);
      if (kExit == warning_type) {
        result = 1;
        break;
      }
    }

    if (!sptk::WriteStream(0, length, mel_cepstrum, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write mel-cepstrum";
      sptk::PrintErrorMessage("mlsacheck", error_message);
      result = 1;
      break;
    }
  }

  delete mlsa_digital_filter_stability_check;
  return result;
}
