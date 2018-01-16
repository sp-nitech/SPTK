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

#include <getopt.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "SPTK/utils/data_symmetrizing.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultFftLength(256);
const sptk::DataSymmetrizing::InputOutputFormats kDefaultInputFormat(
    sptk::DataSymmetrizing::InputOutputFormats::kStandard);
const sptk::DataSymmetrizing::InputOutputFormats kDefaultOutputFormat(
    sptk::DataSymmetrizing::InputOutputFormats::
        kSymmetricForApplyingFourierTransform);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " symmetrize - symmetrize data sequence" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       symmetrize [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : FFT length        (   int)[" << std::setw(5) << std::right << kDefaultFftLength    << "][ 4 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -q q  : input format      (   int)[" << std::setw(5) << std::right << kDefaultInputFormat  << "][ 0 <= q <= 3 ]" << std::endl;  // NOLINT
  *stream << "                 0 ( x(0),     x(1),     ..., x(l/2)                                    )" << std::endl;  // NOLINT
  *stream << "                 1 ( x(0),     x(1),     ..., x(l/2), x(l/2-1), ..., x(1)               )" << std::endl;  // NOLINT
  *stream << "                 2 ( x(l/2)/2, x(l/2-1), ..., x(0),   x(1),     ..., x(l/2-1), x(l/2)/2 )" << std::endl;  // NOLINT
  *stream << "                 3 ( x(l/2),   x(l/2-1), ..., x(0),   x(1),     ..., x(l/2-1), x(l/2)   )" << std::endl;  // NOLINT
  *stream << "       -o o  : output format     (   int)[" << std::setw(5) << std::right << kDefaultOutputFormat << "][ 0 <= o <= 3 ]" << std::endl;  // NOLINT
  *stream << "                 0 ( x(0),     x(1),     ..., x(l/2)                                    )" << std::endl;  // NOLINT
  *stream << "                 1 ( x(0),     x(1),     ..., x(l/2), x(l/2-1), ..., x(1)               )" << std::endl;  // NOLINT
  *stream << "                 2 ( x(l/2)/2, x(l/2-1), ..., x(0),   x(1),     ..., x(l/2-1), x(l/2)/2 )" << std::endl;  // NOLINT
  *stream << "                 3 ( x(l/2),   x(l/2-1), ..., x(0),   x(1),     ..., x(l/2-1), x(l/2)   )" << std::endl;  // NOLINT
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence             (double)[stdin]" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       symmetrized data sequence (double)" << std::endl;
  *stream << "  notice:" << std::endl;
  *stream << "       value of l must be even" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

int main(int argc, char* argv[]) {
  int fft_length(kDefaultFftLength);
  sptk::DataSymmetrizing::InputOutputFormats input_format(kDefaultInputFormat);
  sptk::DataSymmetrizing::InputOutputFormats output_format(
      kDefaultOutputFormat);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:q:o:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &fft_length) ||
            fft_length < 4 || fft_length % 2 == 1) {
          std::ostringstream error_message;
          error_message << "The argument for the -l option must be an even "
                        << "integer larger than 2";
          sptk::PrintErrorMessage("symmetrize", error_message);
          return 1;
        }
        break;
      }
      case 'q': {
        const int min(0);
        const int max(
            static_cast<int>(sptk::DataSymmetrizing::InputOutputFormats::
                                 kNumInputOutputFormats) -
            1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -q option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("symmetrize", error_message);
          return 1;
        }
        input_format =
            static_cast<sptk::DataSymmetrizing::InputOutputFormats>(tmp);
        break;
      }
      case 'o': {
        const int min(0);
        const int max(
            static_cast<int>(sptk::DataSymmetrizing::InputOutputFormats::
                                 kNumInputOutputFormats) -
            1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -o option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("symmetrize", error_message);
          return 1;
        }
        output_format =
            static_cast<sptk::DataSymmetrizing::InputOutputFormats>(tmp);
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
  const int num_rest_args(argc - optind);
  if (1 < num_rest_args) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("symmetrize", error_message);
    return 1;
  }
  const char* input_file(0 == num_rest_args ? NULL : argv[optind]);

  // open stream
  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("symmetrize", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::DataSymmetrizing data_symmetrizing(fft_length, input_format,
                                           output_format);
  if (!data_symmetrizing.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to set condition for symmetrizing";
    sptk::PrintErrorMessage("symmetrize", error_message);
    return 1;
  }

  const int input_length(data_symmetrizing.GetInputLength());
  const int output_length(data_symmetrizing.GetOutputLength());
  std::vector<double> data_sequence(input_length);
  std::vector<double> symmetrized_data_sequence(output_length);

  while (sptk::ReadStream(false, 0, 0, input_length, &data_sequence,
                          &input_stream, NULL)) {
    if (!data_symmetrizing.Run(data_sequence, &symmetrized_data_sequence)) {
      std::ostringstream error_message;
      error_message << "Failed to symmetrize";
      sptk::PrintErrorMessage("symmetrize", error_message);
      return 1;
    }

    if (!sptk::WriteStream(0, output_length, symmetrized_data_sequence,
                           &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write symmetrized data sequence";
      sptk::PrintErrorMessage("symmetrize", error_message);
      return 1;
    }
  }

  return 0;
}
