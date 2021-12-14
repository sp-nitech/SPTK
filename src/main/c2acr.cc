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
#include "SPTK/conversion/cepstrum_to_autocorrelation.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultNumInputOrder(25);
const int kDefaultNumOutputOrder(25);
const int kDefaultFftLength(256);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " c2acr - convert cepstrum to autocorrelation" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       c2acr [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -m m  : order of cepstrum        (   int)[" << std::setw(5) << std::right << kDefaultNumInputOrder  << "][ 0 <= m <  l ]" << std::endl;  // NOLINT
  *stream << "       -M M  : order of autocorrelation (   int)[" << std::setw(5) << std::right << kDefaultNumOutputOrder << "][ 0 <= M <  l ]" << std::endl;  // NOLINT
  *stream << "       -l l  : FFT length               (   int)[" << std::setw(5) << std::right << kDefaultFftLength      << "][ 2 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       cepstrum                         (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       autocorrelation                  (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a c2acr [ @e option ] [ @e infile ]
 *
 * - @b -m @e int
 *   - order of cepstral coefficients @f$(0 \le M_1 < L)@f$
 * - @b -M @e int
 *   - order of autocorrelation coefficients @f$(0 \le M_2 < L)@f$
 * - @b -l @e int
 *   - FFT length @f$(2 \le L)@f$
 * - @b infile @e str
 *   - double-type cepstral coefficients
 * - @b stdout
 *   - double-type autocorrelation coefficients
 *
 * The following example converts the 30-th order cepstral coefficients in
 * @c data.cep into the 15-th order LPC coefficients.
 *
 * @code{.sh}
 *   c2acr -m 30 -M 15 < data.cep | levdur -m 15 > data.lpc
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_input_order(kDefaultNumInputOrder);
  int num_output_order(kDefaultNumOutputOrder);
  int fft_length(kDefaultFftLength);

  for (;;) {
    const char option_char(getopt_long(argc, argv, "m:M:l:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_input_order) ||
            num_input_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("c2acr", error_message);
          return 1;
        }
        break;
      }
      case 'M': {
        if (!sptk::ConvertStringToInteger(optarg, &num_output_order) ||
            num_output_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -M option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("c2acr", error_message);
          return 1;
        }
        break;
      }
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &fft_length)) {
          std::ostringstream error_message;
          error_message << "The argument for the -l option must be an integer";
          sptk::PrintErrorMessage("c2acr", error_message);
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

  // Check arguments.
  if (fft_length <= num_input_order || fft_length <= num_output_order) {
    std::ostringstream error_message;
    error_message
        << "The number of input/output orders must be less than FFT length";
    sptk::PrintErrorMessage("c2acr", error_message);
    return 1;
  }

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("c2acr", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("c2acr", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::CepstrumToAutocorrelation cepstrum_to_autocorrelation(
      num_input_order, num_output_order, fft_length);
  sptk::CepstrumToAutocorrelation::Buffer buffer;
  if (!cepstrum_to_autocorrelation.IsValid()) {
    std::ostringstream error_message;
    error_message << "FFT length must be a power of 2 and greater than 1";
    sptk::PrintErrorMessage("c2acr", error_message);
    return 1;
  }

  const int input_length(num_input_order + 1);
  const int output_length(num_output_order + 1);
  std::vector<double> cepstrum(input_length);
  std::vector<double> autocorrelation(output_length);

  while (sptk::ReadStream(false, 0, 0, input_length, &cepstrum, &input_stream,
                          NULL)) {
    if (!cepstrum_to_autocorrelation.Run(cepstrum, &autocorrelation, &buffer)) {
      std::ostringstream error_message;
      error_message << "Failed to convert cepstrum to autocorrelation";
      sptk::PrintErrorMessage("c2acr", error_message);
      return 1;
    }

    if (!sptk::WriteStream(0, output_length, autocorrelation, &std::cout,
                           NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write autocorrelation coefficients";
      sptk::PrintErrorMessage("c2acr", error_message);
      return 1;
    }
  }

  return 0;
}
