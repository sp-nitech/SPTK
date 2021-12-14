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
#include "SPTK/compression/inverse_uniform_quantization.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum InputFormats { kNonNegativeInteger, kInteger, kNumInputFormats };

const double kDefaultAbsoluteMaximumValue(32768.0);
const int kDefaultNumBit(8);
const sptk::UniformQuantization::QuantizationType kDefaultQuantizationType(
    sptk::UniformQuantization::QuantizationType::kMidRise);
const InputFormats kDefaultInputFormat(kNonNegativeInteger);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " dequantize - inverse uniform quantization" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       dequantize [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -v v  : absolute maximum of input (double)[" << std::setw(5) << std::right << kDefaultAbsoluteMaximumValue << "][ 0.0 <  v <=   ]" << std::endl;  // NOLINT
  *stream << "       -b b  : number of bits per sample (   int)[" << std::setw(5) << std::right << kDefaultNumBit               << "][   1 <= b <=   ]" << std::endl;  // NOLINT
  *stream << "       -t t  : quantization type         (   int)[" << std::setw(5) << std::right << kDefaultQuantizationType     << "][   0 <= t <= 1 ]" << std::endl;  // NOLINT
  *stream << "                 0 (mid-rise)"  << std::endl;
  *stream << "                 1 (mid-tread)"  << std::endl;
  *stream << "       -q q  : input format              (   int)[" << std::setw(5) << std::right << kDefaultInputFormat          << "][   0 <= q <= 1 ]" << std::endl;  // NOLINT
  *stream << "                 0 (non-negative integer)"  << std::endl;
  *stream << "                 1 (integer)"  << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       quantized data sequence           (   int)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       data sequence                     (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a dequantize [ @e option ] [ @e infile ]
 *
 * - @b -v @e double
 *   - absolute maximum value @f$(0 < V)@f$
 * - @b -b @e int
 *   - number of bits @f$(1 \le B)@f$
 * - @b -t @e int
 *   - quantization type
 *     \arg @c 0 mid-rise
 *     \arg @c 1 mid-tread
 * - @b -q @e int
 *   - input format
 *     \arg @c 0 non-negative integer
 *     \arg @c 1 integer
 * - @b infile @e str
 *   - int-type quantized data sequence
 * - @b stdout
 *   - double-type data sequence
 *
 * @code{.sh}
 *   echo -2 -1 0 1 2 | x2x +ad | quantize -b 2 -v 2 -t 0 |
 *     dequantize -b 2 -v 2 -t 0 | x2x +da
 *   # -1.5, -0.5, 0.5, 1.5, 1.5
 *   echo -2 -1 0 1 2 | x2x +ad | quantize -b 2 -v 2 -t 1 |
 *     dequantize -b 2 -v 2 -t 1 | x2x +da
 *   # -1.33333, -1.33333, 0, 1.33333, 1.33333
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  double absolute_maximum_value(kDefaultAbsoluteMaximumValue);
  int num_bit(kDefaultNumBit);
  sptk::UniformQuantization::QuantizationType quantization_type(
      kDefaultQuantizationType);
  InputFormats input_format(kDefaultInputFormat);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "v:b:t:q:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'v': {
        if (!sptk::ConvertStringToDouble(optarg, &absolute_maximum_value) ||
            absolute_maximum_value <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -v option must be a positive number";
          sptk::PrintErrorMessage("dequantize", error_message);
          return 1;
        }
        break;
      }
      case 'b': {
        if (!sptk::ConvertStringToInteger(optarg, &num_bit) || num_bit <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -b option must be a positive integer";
          sptk::PrintErrorMessage("dequantize", error_message);
          return 1;
        }
        break;
      }
      case 't': {
        const int min(0);
        const int max(
            static_cast<int>(sptk::UniformQuantization::QuantizationType::
                                 kNumQuantizationTypes) -
            1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -t option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("dequantize", error_message);
          return 1;
        }
        quantization_type =
            static_cast<sptk::UniformQuantization::QuantizationType>(tmp);
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
          sptk::PrintErrorMessage("dequantize", error_message);
          return 1;
        }
        input_format = static_cast<InputFormats>(tmp);
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
    sptk::PrintErrorMessage("dequantize", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("dequantize", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::InverseUniformQuantization inverse_uniform_quantization(
      absolute_maximum_value, num_bit, quantization_type);
  if (!inverse_uniform_quantization.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize InverseUniformQuantization";
    sptk::PrintErrorMessage("dequantize", error_message);
    return 1;
  }

  const int bias(inverse_uniform_quantization.GetQuantizationLevels() / 2);
  int input;
  double output;

  while (sptk::ReadStream(&input, &input_stream)) {
    if (kInteger == input_format) {
      input += bias;
    }

    if (!inverse_uniform_quantization.Run(input, &output)) {
      std::ostringstream error_message;
      error_message << "Failed to dequantize input";
      sptk::PrintErrorMessage("dequantize", error_message);
      return 1;
    }

    if (!sptk::WriteStream(output, &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write a dequantized sequence";
      sptk::PrintErrorMessage("dequantize", error_message);
      return 1;
    }
  }

  return 0;
}
