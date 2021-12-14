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

#include "Getopt/getoptwin.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kMagicNumberForInfinity(-1);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " impulse - generate impulse sequence" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       impulse [ options ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : output length      (   int)[" << std::setw(5) << std::right << "INF" << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : output order       (   int)[" << std::setw(5) << std::right << "l-1" << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       impulse sequence           (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a impulse [ @e option ]
 *
 * - @b -l @e int
 *   - output length @f$(1 \le L)@f$
 * - @b -m @e int
 *   - output order @f$(0 \le L - 1)@f$
 * - @b stdout
 *   - double-type impulse sequence
 *
 * The output of this command is
 * @f[
 *   \begin{array}{ccccc}
 *     1, & 0, & 0, & \ldots, & 0
 *   \end{array}
 * @f]
 * where the output length is @f$L@f$.
 * If @f$L@f$ is not given, an infinite impulse sequence is generated.
 *
 * In the below example, the impulse response of a ditital filter is calculated
 * and shown on the screen.
 *
 * @code{.sh}
 *   impulse -l 256 | dfs -a 1 0.9 -b 1 2 1 | dmp +d
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int output_length(kMagicNumberForInfinity);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &output_length) ||
            output_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("impulse", error_message);
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
          sptk::PrintErrorMessage("impulse", error_message);
          return 1;
        }
        ++output_length;
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
    sptk::PrintErrorMessage("impulse", error_message);
    return 1;
  }

  if (!sptk::WriteStream(1.0, &std::cout)) {
    std::ostringstream error_message;
    error_message << "Failed to write impulse sequence";
    sptk::PrintErrorMessage("impulse", error_message);
    return 1;
  }

  for (int i(1); kMagicNumberForInfinity == output_length || i < output_length;
       ++i) {
    if (!sptk::WriteStream(0.0, &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write impulse sequence";
      sptk::PrintErrorMessage("impulse", error_message);
      return 1;
    }
  }

  return 0;
}
