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

#include <fstream>   // std::ifstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream
#include <vector>    // std::vector

#include "SPTK/utils/sptk_utils.h"
#include "SPTK/window/data_windowing.h"
#include "SPTK/window/standard_window.h"

namespace {

const int kDefaultFrameLength(256);
const sptk::DataWindowing::NormalizationType kDefaultNormalizationType(
    sptk::DataWindowing::NormalizationType::kPower);
const sptk::StandardWindow::WindowType kDefaultWindowType(
    sptk::StandardWindow::WindowType::kBlackman);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " window - data windowing" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       window [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : frame length of input  (   int)[" << std::setw(5) << std::right << kDefaultFrameLength       << "][ 0 <  l <= L ]" << std::endl;  // NOLINT
  *stream << "       -L L  : frame length of output (   int)[" << std::setw(5) << std::right << "l"                       << "][ l <= L <=   ]" << std::endl;  // NOLINT
  *stream << "       -n n  : normalization type     (   int)[" << std::setw(5) << std::right << kDefaultNormalizationType << "][ 0 <= n <= 2 ]" << std::endl;  // NOLINT
  *stream << "                 0 (none)" << std::endl;
  *stream << "                 1 (power)" << std::endl;
  *stream << "                 2 (magnitude)" << std::endl;
  *stream << "       -w w  : window type            (   int)[" << std::setw(5) << std::right << kDefaultWindowType        << "][ 0 <= w <= 5 ]" << std::endl;  // NOLINT
  *stream << "                 0 (Blackman)" << std::endl;
  *stream << "                 1 (Hamming)" << std::endl;
  *stream << "                 2 (Hanning)" << std::endl;
  *stream << "                 3 (Bartlett)" << std::endl;
  *stream << "                 4 (trapezoidal)" << std::endl;
  *stream << "                 5 (rectangular)" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence                  (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       windowed data sequence         (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a window [ @e option ] [ \e infile ]
 *
 * - @b -l @e int
 *   - input length @f$(1 \le L_1)@f$
 * - @b -L @e int
 *   - output length @f$(1 \le L_2)@f$
 * - @b -n @e int
 *   - normalization type
 *     \arg @c 0 none
 *     \arg @c 1 power
 *     \arg @c 2 magnitude
 * - @b -w @e int
 *   - window type
 *     \arg @c 0 Blackman
 *     \arg @c 1 Hamming
 *     \arg @c 2 Hanning
 *     \arg @c 3 Bartlett
 *     \arg @c 4 Trapezoidal
 *     \arg @c 5 Rectangular
 * - @b infile @e str
 *   - double-type data sequence
 * - @b stdout
 *   - double-type windowed data sequence
 *
 * The below example performs spectral analysis with Blackman window.
 *
 * @code{.sh}
 *   frame -l 400 -p 80 data.d | window -l 400 -L 512 | spec -l 512 > data.spec
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int input_length(kDefaultFrameLength);
  int output_length(kDefaultFrameLength);
  bool is_output_length_specified(false);
  sptk::DataWindowing::NormalizationType normalization_type(
      kDefaultNormalizationType);
  sptk::StandardWindow::WindowType window_type(kDefaultWindowType);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:L:n:w:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &input_length) ||
            input_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("window", error_message);
          return 1;
        }
        break;
      }
      case 'L': {
        if (!sptk::ConvertStringToInteger(optarg, &output_length) ||
            output_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -L option must be a positive integer";
          sptk::PrintErrorMessage("window", error_message);
          return 1;
        }
        is_output_length_specified = true;
        break;
      }
      case 'n': {
        const int min(0);
        const int max(static_cast<int>(sptk::DataWindowing::NormalizationType::
                                           kNumNormalizationTypes) -
                      1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -n option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("window", error_message);
          return 1;
        }
        normalization_type =
            static_cast<sptk::DataWindowing::NormalizationType>(tmp);
        break;
      }
      case 'w': {
        const int min(0);
        const int max(5);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -w option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("window", error_message);
          return 1;
        }
        if (0 == tmp) {
          window_type = sptk::StandardWindow::kBlackman;
        } else if (1 == tmp) {
          window_type = sptk::StandardWindow::kHamming;
        } else if (2 == tmp) {
          window_type = sptk::StandardWindow::kHanning;
        } else if (3 == tmp) {
          window_type = sptk::StandardWindow::kBartlett;
        } else if (4 == tmp) {
          window_type = sptk::StandardWindow::kTrapezoidal;
        } else if (5 == tmp) {
          window_type = sptk::StandardWindow::kRectangular;
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

  if (!is_output_length_specified) {
    output_length = input_length;
  } else if (output_length < input_length) {
    std::ostringstream error_message;
    error_message << "The length of data sequence " << input_length
                  << " must be equal to or less than that of windowed one "
                  << output_length;
    sptk::PrintErrorMessage("window", error_message);
    return 1;
  }

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("window", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("window", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::StandardWindow standard_window(input_length, window_type, false);
  sptk::DataWindowing data_windowing(&standard_window, output_length,
                                     normalization_type);
  if (!data_windowing.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize DataWindowing";
    sptk::PrintErrorMessage("window", error_message);
    return 1;
  }

  std::vector<double> data_sequence(input_length);
  std::vector<double> windowed_data_sequence(output_length);

  while (sptk::ReadStream(false, 0, 0, input_length, &data_sequence,
                          &input_stream, NULL)) {
    if (!data_windowing.Run(data_sequence, &windowed_data_sequence)) {
      std::ostringstream error_message;
      error_message << "Failed to apply a window function";
      sptk::PrintErrorMessage("window", error_message);
      return 1;
    }

    if (!sptk::WriteStream(0, output_length, windowed_data_sequence, &std::cout,
                           NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write windowed data sequence";
      sptk::PrintErrorMessage("window", error_message);
      return 1;
    }
  }

  return 0;
}
