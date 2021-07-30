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
//                1996-2021  Nagoya Institute of Technology          //
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

#include <getopt.h>  // getopt_long

#include <fstream>   // std::ifstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream

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
