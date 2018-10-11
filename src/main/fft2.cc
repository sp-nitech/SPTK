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
//                1996-2018  Nagoya Institute of Technology          //
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
#include <cmath>     // std::sqrt
#include <fstream>   // std::ifstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream

#include "SPTK/math/matrix.h"
#include "SPTK/math/two_dimensional_fast_fourier_transform.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum OutputFormats {
  kOutputRealAndImaginaryParts = 0,
  kOutputRealPart,
  kOutputImaginaryPart,
  kOutputAmplitude,
  kOutputPower,
  kNumOutputFormats
};

enum OutputStyles {
  kStandard = 0,
  kTranspose,
  kTransposeWithBoundary,
  kQuadrantWithBoundary,
  kNumOutputStyles
};

const int kDefaultFftLength(64);
const OutputFormats kDefaultOutputFormat(kOutputRealAndImaginaryParts);
const OutputStyles kDefaultOutputStyle(kStandard);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " fft2 - 2D FFT for complex sequence" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       fft2 [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : FFT length                     (   int)[" << std::setw(5) << std::right << kDefaultFftLength    << "][ 0 <  l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : number of rows                 (   int)[" << std::setw(5) << std::right << "l"                  << "][ 0 <  m <= l ]" << std::endl;  // NOLINT
  *stream << "       -n n  : number of columns              (   int)[" << std::setw(5) << std::right << "l"                  << "][ 0 <  n <= l ]" << std::endl;  // NOLINT
  *stream << "       -o o  : output format                  (   int)[" << std::setw(5) << std::right << kDefaultOutputFormat << "][ 0 <= o <= 4 ]" << std::endl;  // NOLINT
  *stream << "                 0 (real and imaginary parts)" << std::endl;
  *stream << "                 1 (real part)" << std::endl;
  *stream << "                 2 (imaginary part)" << std::endl;
  *stream << "                 3 (amplitude)" << std::endl;
  *stream << "                 4 (power)" << std::endl;
  *stream << "       -p p  : output style                   (   int)[" << std::setw(5) << std::right << kDefaultOutputStyle  << "][ 0 <= p <= 3 ]" << std::endl;  // NOLINT
  *stream << "                 0 (standard)" << std::endl;
  *stream << "                 1 (transpose)" << std::endl;
  *stream << "                 2 (transpose with boundary)" << std::endl;
  *stream << "                 3 (quadrant with boundary)" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence                          (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       2D FFT sequence                        (double)" << std::endl;  // NOLINT
  *stream << "  notice:" << std::endl;
  *stream << "       value of l must be a power of 2" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

int main(int argc, char* argv[]) {
  int fft_length(kDefaultFftLength);
  int num_row(kDefaultFftLength);
  int num_column(kDefaultFftLength);
  bool is_num_row_or_num_column_specified(false);
  OutputFormats output_format(kDefaultOutputFormat);
  OutputStyles output_style(kDefaultOutputStyle);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:n:o:p:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &fft_length)) {
          std::ostringstream error_message;
          error_message << "The argument for the -l option must be an integer";
          sptk::PrintErrorMessage("fft2", error_message);
          return 1;
        }
        break;
      }
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_row) || num_row <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -m option must be a positive integer";
          sptk::PrintErrorMessage("fft2", error_message);
          return 1;
        }
        is_num_row_or_num_column_specified = true;
        break;
      }
      case 'n': {
        if (!sptk::ConvertStringToInteger(optarg, &num_column) ||
            num_column <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -n option must be a positive integer";
          sptk::PrintErrorMessage("fft2", error_message);
          return 1;
        }
        is_num_row_or_num_column_specified = true;
        break;
      }
      case 'o': {
        const int min(0);
        const int max(static_cast<int>(kNumOutputFormats) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -o option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("fft2", error_message);
          return 1;
        }
        output_format = static_cast<OutputFormats>(tmp);
        break;
      }
      case 'p': {
        const int min(0);
        const int max(static_cast<int>(kNumOutputStyles) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -p option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("fft2", error_message);
          return 1;
        }
        output_style = static_cast<OutputStyles>(tmp);
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

  // check arguments
  if (!is_num_row_or_num_column_specified) {
    num_row = fft_length;
    num_column = fft_length;
  } else if (fft_length < num_row || fft_length < num_column) {
    std::ostringstream error_message;
    error_message << "Number of rows and number of columns must be equal to or "
                  << "less than FFT length";
    sptk::PrintErrorMessage("fft2", error_message);
    return 1;
  }

  // get input file
  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("fft2", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  // open stream
  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("fft2", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  // prepare for 2D fast Fourier transform
  sptk::TwoDimensionalFastFourierTransform fast_fourier_transform(
      num_row, num_column, fft_length);
  sptk::TwoDimensionalFastFourierTransform::Buffer buffer;
  if (!fast_fourier_transform.IsValid()) {
    std::ostringstream error_message;
    error_message << "FFT length must be a power of 2";
    sptk::PrintErrorMessage("fft2", error_message);
    return 1;
  }

  const int half_fft_length(fft_length / 2);
  int output_length(0);
  if (kStandard == output_style || kTranspose == output_style) {
    output_length = fft_length;
  } else if (kTransposeWithBoundary == output_style) {
    output_length = fft_length + 1;
  } else if (kQuadrantWithBoundary == output_style) {
    output_length = half_fft_length + 1;
  }
  sptk::Matrix input_x(num_row, num_column);
  sptk::Matrix input_y(num_row, num_column);
  sptk::Matrix tmp_x(fft_length, fft_length);
  sptk::Matrix tmp_y(fft_length, fft_length);
  sptk::Matrix output_x(output_length, output_length);
  sptk::Matrix output_y(output_length, output_length);

  while (sptk::ReadStream(&input_x, &input_stream) &&
         sptk::ReadStream(&input_y, &input_stream)) {
    if (!fast_fourier_transform.Run(input_x, input_y, &tmp_x, &tmp_y,
                                    &buffer)) {
      std::ostringstream error_message;
      error_message << "Failed to run 2D fast Fourier transform";
      sptk::PrintErrorMessage("fft2", error_message);
      return 1;
    }

    if (kStandard == output_style || kQuadrantWithBoundary == output_style) {
      for (int i(0); i < output_length; ++i) {
        for (int j(0); j < output_length; ++j) {
          output_x[i][j] = tmp_x[i][j];
          output_y[i][j] = tmp_y[i][j];
        }
      }
    } else if (kTranspose == output_style ||
               kTransposeWithBoundary == output_style) {
      for (int i(0); i < half_fft_length; ++i) {
        for (int j(0); j < half_fft_length; ++j) {
          output_x[i][j] = tmp_x[i + half_fft_length][j + half_fft_length];
          output_y[i][j] = tmp_y[i + half_fft_length][j + half_fft_length];
        }
      }
      for (int i(half_fft_length); i < fft_length; ++i) {
        for (int j(half_fft_length); j < fft_length; ++j) {
          output_x[i][j] = tmp_x[i - half_fft_length][j - half_fft_length];
          output_y[i][j] = tmp_y[i - half_fft_length][j - half_fft_length];
        }
      }
      for (int i(0); i < half_fft_length; ++i) {
        for (int j(half_fft_length); j < fft_length; ++j) {
          output_x[i][j] = tmp_x[i + half_fft_length][j - half_fft_length];
          output_y[i][j] = tmp_y[i + half_fft_length][j - half_fft_length];
        }
      }
      for (int i(half_fft_length); i < fft_length; ++i) {
        for (int j(0); j < half_fft_length; ++j) {
          output_x[i][j] = tmp_x[i - half_fft_length][j + half_fft_length];
          output_y[i][j] = tmp_y[i - half_fft_length][j + half_fft_length];
        }
      }

      if (kTransposeWithBoundary == output_style) {
        const int boundary(output_length - 1);
        for (int i(0); i < fft_length; ++i) {
          output_x[i][boundary] = output_x[i][0];
          output_x[boundary][i] = output_x[0][i];
          output_y[i][boundary] = output_y[i][0];
          output_y[boundary][i] = output_y[0][i];
        }
        output_x[boundary][boundary] = output_x[0][0];
        output_y[boundary][boundary] = output_y[0][0];
      }
    }

    if (kOutputAmplitude == output_format) {
      for (int i(0); i < output_length; ++i) {
        for (int j(0); j < output_length; ++j) {
          output_x[i][j] = std::sqrt(output_x[i][j] * output_x[i][j] +
                                     output_y[i][j] * output_y[i][j]);
        }
      }
    } else if (kOutputPower == output_format) {
      for (int i(0); i < output_length; ++i) {
        for (int j(0); j < output_length; ++j) {
          output_x[i][j] =
              output_x[i][j] * output_x[i][j] + output_y[i][j] * output_y[i][j];
        }
      }
    }

    if ((kOutputRealAndImaginaryParts == output_format ||
         kOutputRealPart == output_format ||
         kOutputAmplitude == output_format || kOutputPower == output_format) &&
        !sptk::WriteStream(output_x, &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write output sequence";
      sptk::PrintErrorMessage("fft2", error_message);
      return 1;
    }

    if ((kOutputRealAndImaginaryParts == output_format ||
         kOutputImaginaryPart == output_format) &&
        !sptk::WriteStream(output_y, &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write imaginary parts";
      sptk::PrintErrorMessage("fft2", error_message);
      return 1;
    }
  }

  return 0;
}
