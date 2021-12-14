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

#include "Getopt/getoptwin.h"
#include "SPTK/math/matrix.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultNumRow(1);
const int kDefaultNumColumn(1);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " transpose - transpose a matrix" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       transpose [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -r r  : number of rows     (   int)[" << std::setw(5) << std::right << kDefaultNumRow    << "][ 1 <= r <=   ]" << std::endl;  // NOLINT
  *stream << "       -c c  : number of columns  (   int)[" << std::setw(5) << std::right << kDefaultNumColumn << "][ 1 <= c <=   ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence              (double)[stdin]" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       transposed data sequence   (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a transpose [ @e option ] [ @e infile ]
 *
 * - @b -r @e int
 *   - number of rows @f$(1 \le M)@f$
 * - @b -c @e int
 *   - number of columns @f$(1 \le N)@f$
 * - @b infile @e str
 *   - double-type data sequence
 * - @b stdout
 *   - double-type transposed data sequence
 *
 * The input of this command is
 * @f[
 *   \begin{array}{cccc}
 *     x(0,0) & x(0,1) & \cdots & x(0,N-1) \\
 *     x(1,0) & x(1,1) & \cdots & x(1,N-1) \\
 *     \vdots & \vdots &        & \vdots   \\
 *     x(M-1,0) & x(M-1,1) & \cdots & x(M-1,N-1)
 *   \end{array}
 * @f]
 * and the output is
 * @f[
 *   \begin{array}{cccc}
 *     x(0,0) & x(1,0) & \cdots & x(M-1,0) \\
 *     x(0,1) & x(1,1) & \cdots & x(M-1,1) \\
 *     \vdots & \vdots &        & \vdots   \\
 *     x(0,N-1) & x(1,N-1) & \cdots & x(M-1,N-1)
 *   \end{array}
 * @f]
 * where @f$M@f$ is the number of rows and @f$N@f$ is the number of columns.
 * Note that @f$x(m,n) \, \forall m \ge M@f$ are discarded.
 *
 * @code{.sh}
 *   # 0 1 2
 *   # 3 4 5
 *   ramp -l 10 | transpose -r 2 -c 3 | x2x +da -c 2
 *   # 0 3
 *   # 1 4
 *   # 2 5
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_row(kDefaultNumRow);
  int num_column(kDefaultNumColumn);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "r:c:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'r': {
        if (!sptk::ConvertStringToInteger(optarg, &num_row) || num_row <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -r option must be a positive integer";
          sptk::PrintErrorMessage("transpose", error_message);
          return 1;
        }
        break;
      }
      case 'c': {
        if (!sptk::ConvertStringToInteger(optarg, &num_column) ||
            num_column <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -c option must be a positive integer";
          sptk::PrintErrorMessage("transpose", error_message);
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

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("transpose", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("transpose", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::Matrix matrix(num_row, num_column);
  sptk::Matrix transposed_matrix(num_column, num_row);
  while (sptk::ReadStream(&matrix, &input_stream)) {
    if (!matrix.Transpose(&transposed_matrix)) {
      std::ostringstream error_message;
      error_message << "Failed to transpose data";
      sptk::PrintErrorMessage("transpose", error_message);
      return 1;
    }
    if (!sptk::WriteStream(transposed_matrix, &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write transposed data sequence";
      sptk::PrintErrorMessage("transpose", error_message);
      return 1;
    }
  }

  return 0;
}
