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
#include "SPTK/math/discrete_cosine_transform.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum InputFormats {
  kInputRealAndImagParts = 0,
  kInputRealPart,
  kNumInputFormats
};

enum OutputFormats {
  kOutputRealAndImagParts = 0,
  kOutputRealPart,
  kOutputImagPart,
  kOutputAmplitude,
  kOutputPower,
  kNumOutputFormats
};

const int kDefaultDctLength(256);
const InputFormats kDefaultInputFormat(kInputRealPart);
const OutputFormats kDefaultOutputFormat(kOutputRealPart);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " dct - DCT for complex sequence" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       dct [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : DCT length                     (   int)[" << std::setw(5) << std::right << kDefaultDctLength    << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -q q  : input format                   (   int)[" << std::setw(5) << std::right << kDefaultInputFormat  << "][ 0 <= q <= 1 ]" << std::endl;  // NOLINT
  *stream << "                 0 (real and imaginary parts)" << std::endl;
  *stream << "                 1 (real part)" << std::endl;
  *stream << "       -o o  : output format                  (   int)[" << std::setw(5) << std::right << kDefaultOutputFormat << "][ 0 <= o <= 4 ]" << std::endl;  // NOLINT
  *stream << "                 0 (real and imaginary parts)" << std::endl;
  *stream << "                 1 (real part)" << std::endl;
  *stream << "                 2 (imaginary part)" << std::endl;
  *stream << "                 3 (amplitude)" << std::endl;
  *stream << "                 4 (power)" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence                          (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       DCT sequence                           (double)" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a dct [ @e option ] [ @e infile ]
 *
 * - @b -l @e int
 *   - DCT length @f$(1 \le L)@f$
 * - @b -q @e int
 *   - input format
 *     \arg @c 0 real and imaginary parts
 *     \arg @c 1 real part
 * - @b -o @e int
 *   - output format
 *     \arg @c 0 real and imaginary parts
 *     \arg @c 1 real part
 *     \arg @c 2 imaginary part
 *     \arg @c 3 amplitude spectrum
 *     \arg @c 4 power spectrum
 * - @b infile @e str
 *   - double-type data sequence
 * - @b stdout
 *   - double-type DCT sequence
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int dct_length(kDefaultDctLength);
  InputFormats input_format(kDefaultInputFormat);
  OutputFormats output_format(kDefaultOutputFormat);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:q:o:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &dct_length) ||
            dct_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("dct", error_message);
          return 1;
        }
        break;
      }
      case 'q': {
        const int min(0);
        const int max(static_cast<int>(kNumInputFormats) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -q option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("dct", error_message);
          return 1;
        }
        input_format = static_cast<InputFormats>(tmp);
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
          sptk::PrintErrorMessage("dct", error_message);
          return 1;
        }
        output_format = static_cast<OutputFormats>(tmp);
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
    sptk::PrintErrorMessage("dct", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("dct", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::DiscreteCosineTransform discrete_cosine_transform(dct_length);
  sptk::DiscreteCosineTransform::Buffer buffer;
  if (!discrete_cosine_transform.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize DiscreteCosineTransform";
    sptk::PrintErrorMessage("dct", error_message);
    return 1;
  }

  std::vector<double> input_x(dct_length);
  std::vector<double> input_y(dct_length);
  std::vector<double> output_x(dct_length);
  std::vector<double> output_y(dct_length);

  while (
      sptk::ReadStream(true, 0, 0, dct_length, &input_x, &input_stream, NULL)) {
    if (kInputRealAndImagParts == input_format &&
        !sptk::ReadStream(true, 0, 0, dct_length, &input_y, &input_stream,
                          NULL)) {
      break;
    }

    if (!discrete_cosine_transform.Run(input_x, input_y, &output_x, &output_y,
                                       &buffer)) {
      std::ostringstream error_message;
      error_message << "Failed to run discrete cosine transform";
      sptk::PrintErrorMessage("dct", error_message);
      return 1;
    }

    if (kOutputAmplitude == output_format) {
      for (int i(0); i < dct_length; ++i) {
        output_x[i] =
            std::sqrt(output_x[i] * output_x[i] + output_y[i] * output_y[i]);
      }
    } else if (kOutputPower == output_format) {
      for (int i(0); i < dct_length; ++i) {
        output_x[i] = output_x[i] * output_x[i] + output_y[i] * output_y[i];
      }
    }

    if ((kOutputRealAndImagParts == output_format ||
         kOutputRealPart == output_format ||
         kOutputAmplitude == output_format || kOutputPower == output_format) &&
        !sptk::WriteStream(0, dct_length, output_x, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write output sequence";
      sptk::PrintErrorMessage("dct", error_message);
      return 1;
    }

    if ((kOutputRealAndImagParts == output_format ||
         kOutputImagPart == output_format) &&
        !sptk::WriteStream(0, dct_length, output_y, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write imaginary parts";
      sptk::PrintErrorMessage("dct", error_message);
      return 1;
    }
  }

  return 0;
}
