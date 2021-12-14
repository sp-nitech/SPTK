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

#include <cfloat>    // DBL_MAX
#include <fstream>   // std::ifstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream

#include "Getopt/getoptwin.h"
#include "SPTK/math/scalar_operation.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const double kDefaultLowerBound(-DBL_MAX);
const double kDefaultUpperBound(DBL_MAX);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " clip - data clipping" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       clip [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : lower bound        (double)[" << std::setw(5) << std::right << "-INF" << "][   <= l <= u ]" << std::endl;  // NOLINT
  *stream << "       -u u  : upper bound        (double)[" << std::setw(5) << std::right <<  "INF" << "][ l <= u <=   ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence              (double)[stdin]" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       clipped data sequence      (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a clip [ @e option ] [ @e infile ]
 *
 * - @b -l @e double
 *   - lower bound @f$(L)@f$
 * - @b -u @e double
 *   - upper bound @f$(U)@f$
 * - @b infile @e str
 *   - double-type data sequence
 * - @b stdout
 *   - double-type clipped data sequence
 *
 * The output of this command is
 * @f[
 *   f(x) = \left\{ \begin{array}{ll}
 *     L & (x \le L) \\
 *     x & (L < x < U) \\
 *     U & (U \le x)
 *   \end{array} \right.
 * @f]
 * where @f$x@f$ is the input.
 *
 * @code{.sh}
 *   ramp -l 8 | clip -u 4 | x2x +da
 *   # 0, 1, 2, 3, 4, 4, 4, 4
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  double lower_bound(kDefaultLowerBound);
  double upper_bound(kDefaultUpperBound);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:u:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToDouble(optarg, &lower_bound)) {
          std::ostringstream error_message;
          error_message << "The argument for the -l option must be numeric";
          sptk::PrintErrorMessage("clip", error_message);
          return 1;
        }
        break;
      }
      case 'u': {
        if (!sptk::ConvertStringToDouble(optarg, &upper_bound)) {
          std::ostringstream error_message;
          error_message << "The argument for the -u option must be numeric";
          sptk::PrintErrorMessage("clip", error_message);
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

  if (upper_bound < lower_bound) {
    std::ostringstream error_message;
    error_message << "Upper bound must be equal to or greater than lower bound";
    sptk::PrintErrorMessage("clip", error_message);
    return 1;
  }

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("clip", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("clip", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::ScalarOperation scalar_operation;
  if (-DBL_MAX != lower_bound &&
      !scalar_operation.AddLowerBoundingOperation(lower_bound)) {
    std::ostringstream error_message;
    error_message << "Failed to add lower bounding operation";
    sptk::PrintErrorMessage("clip", error_message);
    return 1;
  }
  if (DBL_MAX != upper_bound &&
      !scalar_operation.AddUpperBoundingOperation(upper_bound)) {
    std::ostringstream error_message;
    error_message << "Failed to add upper bounding operation";
    sptk::PrintErrorMessage("clip", error_message);
    return 1;
  }

  double data;
  while (sptk::ReadStream(&data, &input_stream)) {
    bool is_magic_number;
    if (!scalar_operation.Run(&data, &is_magic_number)) {
      std::ostringstream error_message;
      error_message << "Failed to clip data";
      sptk::PrintErrorMessage("clip", error_message);
      return 1;
    }
    if (!sptk::WriteStream(data, &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write clipped data";
      sptk::PrintErrorMessage("clip", error_message);
      return 1;
    }
  }

  return 0;
}
