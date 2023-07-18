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

#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream

#include "GETOPT/ya_getopt.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kMagicNumberForInfinity(-1);
const double kDefaultStepValue(1.0);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " step - generate step sequence" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       step [ options ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : output length      (   int)[" << std::setw(5) << std::right << "INF"             << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : output order       (   int)[" << std::setw(5) << std::right << "l-1"             << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -v v  : step value         (double)[" << std::setw(5) << std::right << kDefaultStepValue << "][   <= v <=   ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       step sequence              (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a step [ @e option ]
 *
 * - @b -l @e int
 *   - output length @f$(1 \le L)@f$
 * - @b -m @e int
 *   - output order @f$(0 \le L - 1)@f$
 * - @b -v @e double
 *   - step value @f$(V)@f$
 * - @b stdout
 *   - double-type step sequence
 *
 * The output of this command is
 * @f[
 *   \begin{array}{cccc}
 *     V, & V, & \ldots, & V
 *   \end{array}
 * @f]
 * where the output length is @f$L@f$.
 * If @f$L@f$ is not given, an infinite step sequence is generated.
 *
 * In the below example, the step response of a ditital filter is calculated and
 * shown on the screen.
 *
 * @code{.sh}
 *   step -l 256 | dfs -a 1 -0.8 | dmp +d
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int output_length(kMagicNumberForInfinity);
  double step_value(kDefaultStepValue);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:v:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &output_length) ||
            output_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("step", error_message);
          return 1;
        }
        break;
      }
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &output_length) ||
            output_length < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                           "non-negative integer";
          sptk::PrintErrorMessage("step", error_message);
          return 1;
        }
        ++output_length;
        break;
      }
      case 'v': {
        if (!sptk::ConvertStringToDouble(optarg, &step_value)) {
          std::ostringstream error_message;
          error_message << "The argument for the -v option must be numeric";
          sptk::PrintErrorMessage("step", error_message);
          return 1;
        }
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

  if (0 != argc - optind) {
    std::ostringstream error_message;
    error_message << "Input file is not required";
    sptk::PrintErrorMessage("step", error_message);
    return 1;
  }

  if (!sptk::SetBinaryMode()) {
    std::ostringstream error_message;
    error_message << "Cannot set translation mode";
    sptk::PrintErrorMessage("step", error_message);
    return 1;
  }

  for (int i(0); kMagicNumberForInfinity == output_length || i < output_length;
       ++i) {
    if (!sptk::WriteStream(step_value, &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write step sequence";
      sptk::PrintErrorMessage("step", error_message);
      return 1;
    }
  }

  return 0;
}
