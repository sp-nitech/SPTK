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
#include <vector>

#include "SPTK/converter/cepstrum_to_negative_derivative_of_phase_spectrum.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum OutputFormats {
  kOutputPoleAndZeroParts = 0,
  kOutputPolePart,
  kOutputZeroPart,
  kNumOutputFormats
};

const int kDefaultNumOrder(25);
const int kDefaultFftLength(256);
const OutputFormats kDefaultOutputFormat(kOutputPoleAndZeroParts);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " c2ndps - transform cepstrum to negative derivative of phase spectrum" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       c2ndps [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -m m  : order of cepstrum             (   int)[" << std::setw(5) << std::right << kDefaultNumOrder     << "][ 0 <= m <= l/2 ]" << std::endl;  // NOLINT
  *stream << "       -l l  : FFT length                    (   int)[" << std::setw(5) << std::right << kDefaultFftLength    << "][ 8 <= l <=     ]" << std::endl;  // NOLINT
  *stream << "       -o o  : output format                 (   int)[" << std::setw(5) << std::right << kDefaultOutputFormat << "][ 0 <= o <= 2   ]" << std::endl;  // NOLINT
  *stream << "                 0 (pole and zero parts)" << std::endl;
  *stream << "                 1 (pole part)" << std::endl;
  *stream << "                 2 (zero part)" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       cepstrum                              (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       negative derivative of phase spectrum (double)" << std::endl;  // NOLINT
  *stream << "  notice:" << std::endl;
  *stream << "       value of l must be a power of 2" << std::endl;
  *stream << "       the output does not contain the information about c(0)" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);
  int fft_length(kDefaultFftLength);
  OutputFormats output_format(kDefaultOutputFormat);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "m:l:o:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("c2ndps", error_message);
          return 1;
        }
        break;
      }
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &fft_length)) {
          std::ostringstream error_message;
          error_message << "The argument for the -l option must be an integer";
          sptk::PrintErrorMessage("c2ndps", error_message);
          return 1;
        }
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
          sptk::PrintErrorMessage("c2ndps", error_message);
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

  // check order
  const int half_fft_length(fft_length / 2);
  if (half_fft_length < num_order) {
    std::ostringstream error_message;
    error_message << "The order of cepstrum " << num_order
                  << " must be equal to or less than the half of FFT length "
                  << half_fft_length;
    sptk::PrintErrorMessage("c2ndps", error_message);
    return 1;
  }

  // get input file
  const int num_rest_args(argc - optind);
  if (1 < num_rest_args) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("c2ndps", error_message);
    return 1;
  }
  const char* input_file(0 == num_rest_args ? NULL : argv[optind]);

  // open stream
  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("c2ndps", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  // prepare for transformation
  sptk::CepstrumToNegativeDerivativeOfPhaseSpectrum
      cepstrum_to_negative_derivative_of_phase_spectrum(num_order, fft_length);
  sptk::CepstrumToNegativeDerivativeOfPhaseSpectrum::Buffer buffer;
  if (!cepstrum_to_negative_derivative_of_phase_spectrum.IsValid()) {
    std::ostringstream error_message;
    error_message << "FFT length must be a power of 2 and greater than 4";
    sptk::PrintErrorMessage("c2ndps", error_message);
    return 1;
  }

  const int input_length(num_order + 1);
  const int output_length(half_fft_length + 1);
  std::vector<double> cepstrum(input_length);
  std::vector<double> negative_derivative_of_phase_spectrum(fft_length);

  while (sptk::ReadStream(false, 0, 0, input_length, &cepstrum, &input_stream,
                          NULL)) {
    if (!cepstrum_to_negative_derivative_of_phase_spectrum.Run(
            cepstrum, &negative_derivative_of_phase_spectrum, &buffer)) {
      std::ostringstream error_message;
      error_message << "Failed to transform cepstrum to negative derivative of "
                    << "phase spectrum";
      sptk::PrintErrorMessage("c2ndps", error_message);
      return 1;
    }

    if (kOutputPolePart == output_format) {
      for (int i(0); i < output_length; ++i) {
        if (negative_derivative_of_phase_spectrum[i] < 0.0) {
          negative_derivative_of_phase_spectrum[i] = 0.0;
        }
      }
    } else if (kOutputZeroPart == output_format) {
      for (int i(0); i < output_length; ++i) {
        if (0.0 < negative_derivative_of_phase_spectrum[i]) {
          negative_derivative_of_phase_spectrum[i] = 0.0;
        }
      }
    }

    if (!sptk::WriteStream(0, output_length,
                           negative_derivative_of_phase_spectrum, &std::cout,
                           NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write negative derivative of phase spectrum";
      sptk::PrintErrorMessage("c2ndps", error_message);
      return 1;
    }
  }

  return 0;
}
