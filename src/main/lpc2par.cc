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
#include "SPTK/conversion/linear_predictive_coefficients_to_parcor_coefficients.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum WarningType { kIgnore = 0, kWarn, kExit, kNumWarningTypes };

const int kDefaultNumOrder(25);
const double kDefaultGamma(1.0);
const WarningType kDefaultWarningType(kIgnore);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " lpc2par - convert linear predictive coefficients to PARCOR coefficients" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       lpc2par [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -m m  : order of coefficients                  (   int)[" << std::setw(5) << std::right << kDefaultNumOrder    << "][    0 <= m <=     ]" << std::endl;  // NOLINT
  *stream << "       -g g  : gamma of generalized cepstrum          (double)[" << std::setw(5) << std::right << kDefaultGamma       << "][ -1.0 <= g <= 1.0 ]" << std::endl;  // NOLINT
  *stream << "       -c c  : gamma of generalized cepstrum = -1 / c (   int)[" << std::setw(5) << std::right << "N/A"               << "][    1 <= c <=     ]" << std::endl;  // NOLINT
  *stream << "       -e e  : warning type of unstable index         (   int)[" << std::setw(5) << std::right << kDefaultWarningType << "][    0 <= e <= 2   ]" << std::endl;  // NOLINT
  *stream << "                 0 (no warning)" << std::endl;
  *stream << "                 1 (output the index to stderr)" << std::endl;
  *stream << "                 2 (output the index to stderr" << std::endl;
  *stream << "                    and exit immediately)" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       linear predictive coefficients                 (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       PARCOR coefficients                            (double)" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a lpc2par [ @e option ] [ @e infile ]
 *
 * - @b -m @e int
 *   - order of coefficients @f$(0 \le M)@f$
 * - @b -g @e double
 *   - gamma @f$(|\gamma| \le 1)@f$
 * - @b -c @e double
 *   - gamma @f$\gamma = -1 / C@f$ @f$(1 \le C)@f$
 * - @b -e @e int
 *   - type of warning of unstable coefficients
 *     \arg @c 0 no warning
 *     \arg @c 1 output the index to stderr
 *     \arg @c 2 output the index to stderr and exit immediately
 * - @b infile @e str
 *   - double-type LPC coefficients
 * - @b stdout
 *   - double-type PARCOR coefficients
 *
 * The below example extracts PARCOR coefficients from @c data.d
 *
 * @code{.sh}
 *   frame < data.d | window | lpc | lpc2par > data.rc
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);
  double gamma(kDefaultGamma);
  WarningType warning_type(kDefaultWarningType);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "m:g:c:e:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("lpc2par", error_message);
          return 1;
        }
        break;
      }
      case 'g': {
        if (!sptk::ConvertStringToDouble(optarg, &gamma) ||
            !sptk::IsValidGamma(gamma)) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -g option must be in [-1.0, 1.0]";
          sptk::PrintErrorMessage("lpc2par", error_message);
          return 1;
        }
        break;
      }
      case 'c': {
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) || tmp < 1) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -c option must be a positive integer";
          sptk::PrintErrorMessage("lpc2par", error_message);
          return 1;
        }
        gamma = -1.0 / tmp;
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
          sptk::PrintErrorMessage("lpc2par", error_message);
          return 1;
        }
        warning_type = static_cast<WarningType>(tmp);
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

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("lpc2par", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("lpc2par", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::LinearPredictiveCoefficientsToParcorCoefficients
      linear_predictive_coefficients_to_parcor_coefficients(num_order, gamma);
  sptk::LinearPredictiveCoefficientsToParcorCoefficients::Buffer buffer;
  if (!linear_predictive_coefficients_to_parcor_coefficients.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize "
                     "LinearPredictiveCoefficientsToParcorCoefficients";
    sptk::PrintErrorMessage("lpc2par", error_message);
    return 1;
  }

  const int length(num_order + 1);
  std::vector<double> coefficients(length);

  for (int frame_index(0); sptk::ReadStream(false, 0, 0, length, &coefficients,
                                            &input_stream, NULL);
       ++frame_index) {
    bool is_stable(false);
    if (!linear_predictive_coefficients_to_parcor_coefficients.Run(
            &coefficients, &is_stable, &buffer)) {
      std::ostringstream error_message;
      error_message << "Failed to convert linear predictive coefficients to "
                    << "PARCOR coefficients";
      sptk::PrintErrorMessage("lpc2par", error_message);
      return 1;
    }

    if (!is_stable && kIgnore != warning_type) {
      std::ostringstream error_message;
      error_message << frame_index << "th frame is unstable";
      sptk::PrintErrorMessage("lpc2par", error_message);
      if (kExit == warning_type) return 1;
    }

    if (!sptk::WriteStream(0, length, coefficients, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write PARCOR coefficients";
      sptk::PrintErrorMessage("lpc2par", error_message);
      return 1;
    }
  }

  return 0;
}
