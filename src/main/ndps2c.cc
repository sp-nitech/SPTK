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
#include "SPTK/conversion/negative_derivative_of_phase_spectrum_to_cepstrum.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultFftLength(256);
const int kDefaultNumOrder(25);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " ndps2c - transform negative derivative of phase spectrum to cepstrum" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       ndps2c [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : FFT length                    (   int)[" << std::setw(5) << std::right << kDefaultFftLength << "][ 2 <= l <=     ]" << std::endl;  // NOLINT
  *stream << "       -m m  : order of cepstrum             (   int)[" << std::setw(5) << std::right << kDefaultNumOrder  << "][ 0 <= m <= l/2 ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       negative derivative of phase spectrum (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       cepstrum                              (double)" << std::endl;  // NOLINT
  *stream << "  notice:" << std::endl;
  *stream << "       value of l must be a power of 2" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a ndps2c [ @e option ] [ @e infile ]
 *
 * - @b -l @e int
 *   - FFT length @f$(2 \le L)@f$
 * - @b -m @e int
 *   - order of cepstrum @f$(0 \le M \le L/2)@f$
 * - @b infile @e str
 *   - double-type NDPS
 * - @b stdout
 *   - double-type cepstrum
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int fft_length(kDefaultFftLength);
  int num_order(kDefaultNumOrder);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &fft_length)) {
          std::ostringstream error_message;
          error_message << "The argument for the -l option must be an integer";
          sptk::PrintErrorMessage("ndps2c", error_message);
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
          sptk::PrintErrorMessage("ndps2c", error_message);
          return 1;
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

  const int half_fft_length(fft_length / 2);
  if (half_fft_length < num_order) {
    std::ostringstream error_message;
    error_message << "The order of cepstrum " << num_order
                  << " must be equal or less than the half of FFT length "
                  << half_fft_length;
    sptk::PrintErrorMessage("ndps2c", error_message);
    return 1;
  }

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("ndps2c", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("ndps2c", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::NegativeDerivativeOfPhaseSpectrumToCepstrum
      negative_derivative_of_phase_spectrum_to_cepstrum(fft_length, num_order);
  sptk::NegativeDerivativeOfPhaseSpectrumToCepstrum::Buffer buffer;
  if (!negative_derivative_of_phase_spectrum_to_cepstrum.IsValid()) {
    std::ostringstream error_message;
    error_message
        << "Failed to initialize NegativeDerivativeOfPhaseSpectrumToCepstrum";
    sptk::PrintErrorMessage("ndps2c", error_message);
    return 1;
  }

  const int input_length(half_fft_length + 1);
  const int output_length(num_order + 1);
  std::vector<double> negative_derivative_of_phase_spectrum(input_length);
  std::vector<double> cepstrum(output_length);

  while (sptk::ReadStream(false, 0, 0, input_length,
                          &negative_derivative_of_phase_spectrum, &input_stream,
                          NULL)) {
    if (!negative_derivative_of_phase_spectrum_to_cepstrum.Run(
            negative_derivative_of_phase_spectrum, &cepstrum, &buffer)) {
      std::ostringstream error_message;
      error_message << "Failed to transform negative derivative of phase "
                    << "spectrum to cepstrum";
      sptk::PrintErrorMessage("ndps2c", error_message);
      return 1;
    }

    if (!sptk::WriteStream(0, output_length, cepstrum, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write cepstrum";
      sptk::PrintErrorMessage("ndps2c", error_message);
      return 1;
    }
  }

  return 0;
}
