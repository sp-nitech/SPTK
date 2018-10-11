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
#include <fstream>   // std::ifstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream

#include "SPTK/quantizer/inverse_uniform_quantization.h"
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
  *stream << "       input sequence                    (   int)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       dequantized sequence              (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

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

  // get input file
  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("dequantize", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  // open stream
  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("dequantize", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  // prepare for dequantization
  sptk::InverseUniformQuantization inverse_uniform_quantization(
      absolute_maximum_value, num_bit, quantization_type);
  if (!inverse_uniform_quantization.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to set condition for dequantization";
    sptk::PrintErrorMessage("dequantize", error_message);
    return 1;
  }

  int input;
  double output;

  while (sptk::ReadStream(&input, &input_stream)) {
    if (kInteger == input_format) {
      input += inverse_uniform_quantization.GetQuantizationLevels() / 2;
    }

    if (!inverse_uniform_quantization.Run(input, &output)) {
      std::ostringstream error_message;
      error_message << "Failed to dequantize";
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
