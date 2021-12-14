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

#include <cmath>     // std::fabs
#include <fstream>   // std::ifstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream

#include "Getopt/getoptwin.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum ErrorTypes { kAbsolute = 0, kRelative, kNumErrorTypes };

const double kDefaultTolerance(1e-6);
const ErrorTypes kDefaultErrorType(kAbsolute);
const bool kDefaultEnableCheckLengthFlag(true);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " aeq - check almost equality" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       aeq [ options ] exfile [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -t t  : tolerance               (double)[" << std::setw(5) << std::right << kDefaultTolerance << "][ 0.0 <= t <=   ]" << std::endl;  // NOLINT
  *stream << "       -e e  : error type              (   int)[" << std::setw(5) << std::right << kDefaultErrorType << "][   0 <= e <= 1 ]" << std::endl;  // NOLINT
  *stream << "                 0 (absolute error)" << std::endl;
  *stream << "                 1 (relative error)" << std::endl;
  *stream << "       -L    : disable to check length (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(!kDefaultEnableCheckLengthFlag) << "]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  exfile:" << std::endl;
  *stream << "       expected values                 (double)" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       actual values                   (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       result messages" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a aeq [ @e option ] @e exfile [ @e infile ]
 *
 * - @b -t @e double
 *   - absolute tolerance @f$(0 \le \epsilon)@f$
 * - @b -e @e int
 *   - error type
 *     \arg @c 0 absolute error
 *     \arg @c 1 relative error
 * - @b -L @e bool
 *   - disable to check length
 * - @b exfile @e str
 *   - double-type expected values
 * - @b infile @e str
 *   - double-type actual values
 * - @b stdout
 *   - result messages
 *
 * This command checks whether two data sequences are almost equal or not.
 *
 * Given the two data sequences
 * @f[
 *   \begin{array}{cccc}
 *     x_1(0), & x_1(1), & \ldots, & x_1(T_1), \\
 *     x_2(0), & x_2(1), & \ldots, & x_2(T_2), \\
 *   \end{array}
 * @f]
 * the absolute error @f$e(t)@f$ is calculated sample-by-sample:
 * @f[
 *   e(t) = | x_1(t) - x_2(t) |.
 * @f]
 * If @f$e(t)@f$ is greater than tolerance @f$\epsilon@f$ or @f$T_1@f$ is not
 * equal to @f$T_2@f$, the command prints a warning message.
 *
 * The below example checks the equality between two data:
 *
 * @code{.sh}
 *   aeq -t 0 data.x1 data.x2
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @retval -1 Given two data sequences are not almost equal.
 * @retval 0 Given two data sequences are almost equal.
 * @retval 1 Failed to run this command.
 */
int main(int argc, char* argv[]) {
  double tolerance(kDefaultTolerance);
  ErrorTypes error_type(kDefaultErrorType);
  bool enable_check_length(kDefaultEnableCheckLengthFlag);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "t:e:Lh", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 't': {
        if (!sptk::ConvertStringToDouble(optarg, &tolerance) ||
            tolerance < 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -t option must be a non-negative number";
          sptk::PrintErrorMessage("aeq", error_message);
          return 1;
        }
        break;
      }
      case 'e': {
        const int min(0);
        const int max(static_cast<int>(kNumErrorTypes) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -e option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("aeq", error_message);
          return 1;
        }
        error_type = static_cast<ErrorTypes>(tmp);
        break;
        break;
      }
      case 'L': {
        enable_check_length = false;
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

  const char* expected_file;
  const char* actual_file;
  const int num_input_files(argc - optind);
  if (2 == num_input_files) {
    expected_file = argv[argc - 2];
    actual_file = argv[argc - 1];
  } else if (1 == num_input_files) {
    expected_file = argv[argc - 1];
    actual_file = NULL;
  } else {
    std::ostringstream error_message;
    error_message << "Just two input files, exfile and infile, are required";
    sptk::PrintErrorMessage("aeq", error_message);
    return 1;
  }

  std::ifstream ifs1;
  ifs1.open(expected_file, std::ios::in | std::ios::binary);
  if (ifs1.fail()) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << expected_file;
    sptk::PrintErrorMessage("aeq", error_message);
    return 1;
  }
  std::istream& stream_for_expected(ifs1);

  std::ifstream ifs2;
  ifs2.open(actual_file, std::ios::in | std::ios::binary);
  if (ifs2.fail() && NULL != actual_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << actual_file;
    sptk::PrintErrorMessage("aeq", error_message);
    return 1;
  }
  std::istream& stream_for_actual(ifs2.fail() ? std::cin : ifs2);

  double expected;
  double actual;
  int status(0);

  for (int sample_index(0);; ++sample_index) {
    const bool result1(sptk::ReadStream(&expected, &stream_for_expected));
    const bool result2(sptk::ReadStream(&actual, &stream_for_actual));
    if (result1 != result2) {
      if (enable_check_length) {
        std::cout << "Acutual data length and expected one differ" << std::endl;
        status = -1;
      }
      break;
    }
    if (!result1) {
      break;
    }

    double error(std::fabs(actual - expected));
    if (kRelative == error_type) {
      error /= std::fabs(expected);
    }
    if (tolerance < error) {
      std::cout << "[No. " << sample_index << "] is not almost equal "
                << "(Error: " << error << ")" << std::endl;
      status = -1;
    }
  }

  return status;
}
