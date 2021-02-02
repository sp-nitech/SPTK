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
//                1996-2020  Nagoya Institute of Technology          //
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

#include <algorithm>  // std::reverse
#include <fstream>    // std::ifstream
#include <iomanip>    // std::setw
#include <iostream>   // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>    // std::ostringstream
#include <vector>     // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace {

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " reverse - reverse the order of data in each block" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       reverse [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : block length       (   int)[" << std::setw(5) << std::right << "EOF" << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : block order        (   int)[" << std::setw(5) << std::right << "l-1" << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence              (double)[stdin]" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       reversed data sequence     (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a reverse [ @e option ] [ @e infile ]
 *
 * - @b -l @e int
 *   - block length @f$(1 \le L)@f$
 * - @b -m @e int
 *   - block order @f$(0 \le L - 1)@f$
 * - @b infile @e str
 *   - double-type data sequence
 * - @b stdout
 *   - double-type reversed data sequence
 *
 * The input of this command is
 * @f[
 *   \begin{array}{ccc}
 *     \underbrace{x_0(0), \; \ldots, \; x_0(L-1)}_L, &
 *     \underbrace{x_1(0), \; \ldots, \; x_1(L-1)}_L, &
 *     \ldots,
 *   \end{array}
 * @f]
 * and the output is
 * @f[
 *   \begin{array}{ccc}
 *     \underbrace{x_0(L-1), \; \ldots, \; x_0(0)}_L, &
 *     \underbrace{x_1(L-1), \; \ldots, \; x_1(0)}_L, &
 *     \ldots,
 *   \end{array}
 * @f]
 * where @f$L@f$ is the block length. If @f$L@f$ is not given, @f$L@f$ is
 * assumed to be the length of entire sequence.
 *
 * @code{.sh}
 *   ramp -l 9 | reverse | x2x +da
 *   # 8, 7, 6, 5, 4, 3, 2, 1, 0
 * @endcode
 *
 * @code{.sh}
 *   ramp -l 9 | reverse -l 3 | x2x +da
 *   # 2, 1, 0, 5, 4, 3, 8, 7, 6
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int block_length(0);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &block_length) ||
            block_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("reverse", error_message);
          return 1;
        }
        break;
      }
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &block_length) ||
            block_length < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("reverse", error_message);
          return 1;
        }
        ++block_length;
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
    sptk::PrintErrorMessage("reverse", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("reverse", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  if (0 == block_length) {
    // Reverse whole sequence.
    std::vector<double> data;
    double tmp;
    while (sptk::ReadStream(&tmp, &input_stream)) {
      data.push_back(tmp);
    }
    if (!data.empty()) {
      std::reverse(data.begin(), data.end());
      if (!sptk::WriteStream(0, data.size(), data, &std::cout, NULL)) {
        std::ostringstream error_message;
        error_message << "Failed to write reversed data sequence";
        sptk::PrintErrorMessage("reverse", error_message);
        return 1;
      }
    }
  } else {
    // Reverse partial sequence.
    std::vector<double> data(block_length);
    while (sptk::ReadStream(false, 0, 0, block_length, &data, &input_stream,
                            NULL)) {
      std::reverse(data.begin(), data.end());
      if (!sptk::WriteStream(0, block_length, data, &std::cout, NULL)) {
        std::ostringstream error_message;
        error_message << "Failed to write reversed data sequence";
        sptk::PrintErrorMessage("reverse", error_message);
        return 1;
      }
    }
  }

  return 0;
}
