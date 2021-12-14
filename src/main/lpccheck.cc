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
#include "SPTK/check/linear_predictive_coefficients_stability_check.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum WarningType { kIgnore = 0, kWarn, kExit, kNumWarningTypes };

const int kDefaultNumOrder(25);
const WarningType kDefaultWarningType(kWarn);
const double kDefaultMargin(1e-16);
const bool kDefaultModificationFlag(false);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " lpccheck - check stability of linear predictive coefficients" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       lpccheck [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -m m  : order of coefficients           (   int)[" << std::setw(5) << std::right << kDefaultNumOrder    << "][   0 <= m <=    ]" << std::endl;  // NOLINT
  *stream << "       -e e  : warning type of unstable index  (   int)[" << std::setw(5) << std::right << kDefaultWarningType << "][   0 <= e <= 2  ]" << std::endl;  // NOLINT
  *stream << "                 0 (no warning)" << std::endl;
  *stream << "                 1 (output the index to stderr)" << std::endl;
  *stream << "                 2 (output the index to stderr" << std::endl;
  *stream << "                    and exit immediately)" << std::endl;
  *stream << "       -r r  : margin                          (double)[" << std::setw(5) << std::right << kDefaultMargin      << "][ 0.0 <  r < 1.0 ]" << std::endl;  // NOLINT
  *stream << "       -x    : perform modification            (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultModificationFlag) << "]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       linear predictive coefficients          (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       modified linear predictive coefficients (double)" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a lpccheck [ @e option ] [ @e infile ]
 *
 * - @b -m @e int
 *   - order of linear predictive coefficients @f$(0 \le M)@f$
 * - @b -e @e int
 *   - warning type
 *     \arg @c 0 no warning
 *     \arg @c 1 output index
 *     \arg @c 2 output index and exit immediately
 * - @b -r @e double
 *   - margin @f$(0 < \delta < 1)@f$
 * - @b -x @e bool
 *   - perform modification
 * - @b infile @e str
 *   - double-type LPC coefficients
 * - @b stdout
 *   - double-type modified LPC coefficients
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);
  WarningType warning_type(kDefaultWarningType);
  double margin(kDefaultMargin);
  bool modification_flag(kDefaultModificationFlag);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "m:e:r:xh", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("lpccheck", error_message);
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
          sptk::PrintErrorMessage("lpccheck", error_message);
          return 1;
        }
        warning_type = static_cast<WarningType>(tmp);
        break;
      }
      case 'r': {
        if (!sptk::ConvertStringToDouble(optarg, &margin) || margin <= 0.0 ||
            1.0 <= margin) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -r option must be in (0.0, 1.0)";
          sptk::PrintErrorMessage("lpccheck", error_message);
          return 1;
        }
        break;
      }
      case 'x': {
        modification_flag = true;
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

  const int num_rest_args(argc - optind);
  if (1 < num_rest_args) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("lpccheck", error_message);
    return 1;
  }
  const char* input_file(0 == num_rest_args ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("lpccheck", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::LinearPredictiveCoefficientsStabilityCheck
      linear_predictive_coefficients_stability_check(num_order, margin);
  sptk::LinearPredictiveCoefficientsStabilityCheck::Buffer buffer;
  if (!linear_predictive_coefficients_stability_check.IsValid()) {
    std::ostringstream error_message;
    error_message
        << "Failed to initialize LinearPredictiveCoefficientsStabilityCheck";
    sptk::PrintErrorMessage("lpccheck", error_message);
    return 1;
  }

  const int length(num_order + 1);
  std::vector<double> linear_predictive_coefficients(length);

  for (int frame_index(0);
       sptk::ReadStream(false, 0, 0, length, &linear_predictive_coefficients,
                        &input_stream, NULL);
       ++frame_index) {
    bool is_stable(false);
    if (modification_flag) {
      if (!linear_predictive_coefficients_stability_check.Run(
              &linear_predictive_coefficients, &is_stable, &buffer)) {
        std::ostringstream error_message;
        error_message
            << "Failed to check stability of linear predictive coefficients";
        sptk::PrintErrorMessage("lpccheck", error_message);
        return 1;
      } else {
        if (!linear_predictive_coefficients_stability_check.Run(
                linear_predictive_coefficients, NULL, &is_stable, &buffer)) {
          std::ostringstream error_message;
          error_message
              << "Failed to check stability of linear predictive coefficients";
          sptk::PrintErrorMessage("lpccheck", error_message);
          return 1;
        }
      }
    }

    if (!is_stable && kIgnore != warning_type) {
      std::ostringstream error_message;
      error_message << frame_index << "th frame is unstable";
      sptk::PrintErrorMessage("lpccheck", error_message);
      if (kExit == warning_type) return 1;
    }

    if (!sptk::WriteStream(0, length, linear_predictive_coefficients,
                           &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write linear predictive coefficients";
      sptk::PrintErrorMessage("lpccheck", error_message);
      return 1;
    }
  }

  return 0;
}
