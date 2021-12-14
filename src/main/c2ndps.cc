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
#include <vector>    // std::vector

#include "Getopt/getoptwin.h"
#include "SPTK/conversion/cepstrum_to_negative_derivative_of_phase_spectrum.h"
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
  *stream << "       -l l  : FFT length                    (   int)[" << std::setw(5) << std::right << kDefaultFftLength    << "][ 2 <= l <=     ]" << std::endl;  // NOLINT
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
  *stream << "       c(0) is not used in the calculation" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a c2ndps [ @e option ] [ @e infile ]
 *
 * - @b -m @e int
 *   - order of cepstrum @f$(0 \le M \le L/2)@f$
 * - @b -l @e int
 *   - FFT length @f$(2 \le L)@f$
 * - @b infile @e str
 *   - double-type cepstrum
 * - @b stdout
 *   - double-type NDPS
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
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

  const int half_fft_length(fft_length / 2);
  if (half_fft_length < num_order) {
    std::ostringstream error_message;
    error_message << "The order of cepstrum " << num_order
                  << " must be equal to or less than the half of FFT length "
                  << half_fft_length;
    sptk::PrintErrorMessage("c2ndps", error_message);
    return 1;
  }

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("c2ndps", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("c2ndps", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::CepstrumToNegativeDerivativeOfPhaseSpectrum
      cepstrum_to_negative_derivative_of_phase_spectrum(num_order, fft_length);
  sptk::CepstrumToNegativeDerivativeOfPhaseSpectrum::Buffer buffer;
  if (!cepstrum_to_negative_derivative_of_phase_spectrum.IsValid()) {
    std::ostringstream error_message;
    error_message
        << "Failed to initialize CepstrumToNegativeDerivativeOfPhaseSpectrum";
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
