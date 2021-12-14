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

#include <cmath>     // std::sqrt
#include <fstream>   // std::ifstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream
#include <vector>    // std::vector

#include "Getopt/getoptwin.h"
#include "SPTK/math/real_valued_fast_fourier_transform.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum OutputFormats {
  kOutputRealAndImagParts = 0,
  kOutputRealPart,
  kOutputImagPart,
  kOutputAmplitude,
  kOutputPower,
  kNumOutputFormats
};

const int kDefaultFftLength(256);
const OutputFormats kDefaultOutputFormat(kOutputRealAndImagParts);
const bool kDefaultOutputHalfPartFlag(false);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " fftr - FFT for real sequence" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       fftr [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : FFT length                     (   int)[" << std::setw(5) << std::right << kDefaultFftLength    << "][ 2 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : order of sequence              (   int)[" << std::setw(5) << std::right << "l-1"                << "][ 0 <= m <  l ]" << std::endl;  // NOLINT
  *stream << "       -o o  : output format                  (   int)[" << std::setw(5) << std::right << kDefaultOutputFormat << "][ 0 <= o <= 4 ]" << std::endl;  // NOLINT
  *stream << "                 0 (real and imaginary parts)" << std::endl;
  *stream << "                 1 (real part)" << std::endl;
  *stream << "                 2 (imaginary part)" << std::endl;
  *stream << "                 3 (amplitude)" << std::endl;
  *stream << "                 4 (power)" << std::endl;
  *stream << "       -H    : output only half part          (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultOutputHalfPartFlag) << "]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence                          (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       FFT sequence                           (double)" << std::endl;  // NOLINT
  *stream << "  notice:" << std::endl;
  *stream << "       value of l must be a power of 2" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a fftr [ @e option ] [ @e infile ]
 *
 * - @b -l @e int
 *   - FFT length @f$(2 \le L)@f$
 * - @b -m @e int
 *   - order of sequence @f$(0 \le M < L)@f$
 * - @b -o @e int
 *   - output format
 *     \arg @c 0 real and imaginary parts
 *     \arg @c 1 real part
 *     \arg @c 2 imaginary part
 *     \arg @c 3 amplitude spectrum
 *     \arg @c 4 power spectrum
 * - @b -H @e bool
 *   - output only half part
 * - @b infile @e str
 *   - double-type data sequence
 * - @b stdout
 *   - double-type FFT sequence
 *
 * The below example analyzes a sine wave using Blackman window.
 *
 * @code{.sh}
 *   sin -p 30 -l 256 | window | fftr -o 3 > sine.spec
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int fft_length(kDefaultFftLength);
  int num_order(kDefaultFftLength - 1);
  bool is_num_order_specified(false);
  OutputFormats output_format(kDefaultOutputFormat);
  bool output_half_part_flag(kDefaultOutputHalfPartFlag);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:o:Hh", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &fft_length)) {
          std::ostringstream error_message;
          error_message << "The argument for the -l option must be an integer";
          sptk::PrintErrorMessage("fftr", error_message);
          return 1;
        }
        break;
      }
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("fftr", error_message);
          return 1;
        }
        is_num_order_specified = true;
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
          sptk::PrintErrorMessage("fftr", error_message);
          return 1;
        }
        output_format = static_cast<OutputFormats>(tmp);
        break;
      }
      case 'H': {
        output_half_part_flag = true;
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

  if (!is_num_order_specified) {
    num_order = fft_length - 1;
  } else if (fft_length <= num_order) {
    std::ostringstream error_message;
    error_message << "The order of data sequence " << num_order
                  << " must be less than FFT length " << fft_length;
    sptk::PrintErrorMessage("fftr", error_message);
    return 1;
  }

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("fftr", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("fftr", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::RealValuedFastFourierTransform fast_fourier_transform(num_order,
                                                              fft_length);
  sptk::RealValuedFastFourierTransform::Buffer buffer;
  if (!fast_fourier_transform.IsValid()) {
    std::ostringstream error_message;
    error_message << "FFT length must be a power of 2 and greater than 1";
    sptk::PrintErrorMessage("fftr", error_message);
    return 1;
  }

  const int input_length(num_order + 1);
  const int output_length(output_half_part_flag ? (fft_length / 2 + 1)
                                                : fft_length);
  std::vector<double> input_x(input_length);
  std::vector<double> output_x(fft_length);
  std::vector<double> output_y(fft_length);

  while (sptk::ReadStream(true, 0, 0, input_length, &input_x, &input_stream,
                          NULL)) {
    if (!fast_fourier_transform.Run(input_x, &output_x, &output_y, &buffer)) {
      std::ostringstream error_message;
      error_message << "Failed to run fast Fourier transform";
      sptk::PrintErrorMessage("fftr", error_message);
      return 1;
    }

    if (kOutputAmplitude == output_format) {
      for (int i(0); i < output_length; ++i) {
        output_x[i] =
            std::sqrt(output_x[i] * output_x[i] + output_y[i] * output_y[i]);
      }
    } else if (kOutputPower == output_format) {
      for (int i(0); i < output_length; ++i) {
        output_x[i] = output_x[i] * output_x[i] + output_y[i] * output_y[i];
      }
    }

    if (kOutputImagPart != output_format &&
        !sptk::WriteStream(0, output_length, output_x, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write output sequence";
      sptk::PrintErrorMessage("fftr", error_message);
      return 1;
    }

    if ((kOutputRealAndImagParts == output_format ||
         kOutputImagPart == output_format) &&
        !sptk::WriteStream(0, output_length, output_y, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write imaginary parts";
      sptk::PrintErrorMessage("fftr", error_message);
      return 1;
    }
  }

  return 0;
}
