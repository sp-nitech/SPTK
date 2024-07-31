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

#include "GETOPT/ya_getopt.h"
#include "SPTK/conversion/mel_cepstrum_power_normalization.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultNumOrder(25);
const int kDefaultImpulseResponseLength(128);
const double kDefaultAlpha(0.35);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " pnorm - power normalization of mel-cepstrum" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       pnorm [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -m m  : order of mel-cepstrum      (   int)[" << std::setw(5) << std::right << kDefaultNumOrder              << "][    0 <= m <      ]" << std::endl;  // NOLINT
  *stream << "       -l l  : length of impulse response (   int)[" << std::setw(5) << std::right << kDefaultImpulseResponseLength << "][    2 <= l <=     ]" << std::endl;  // NOLINT
  *stream << "       -a a  : all-pass constant          (double)[" << std::setw(5) << std::right << kDefaultAlpha                 << "][ -1.0 <  a <  1.0 ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       mel-cepstrum                       (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       power-normalized mel-cepstrum      (double)" << std::endl;
  *stream << "  notice:" << std::endl;
  *stream << "       value of l must be a power of 2" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a pnorm [ @e option ] [ @e infile ]
 *
 * - @b -m @e int
 *   - order of mel-cepstral coefficients @f$(0 \le M)@f$
 * - @b -l @e int
 *   - length of impulse response @f$(2 \le L)@f$
 * - @b -a @e double
 *   - alpha @f$(|\alpha|<1)@f$
 * - @b infile @e str
 *   - double-type mel-cepstral coefficients
 * - @b stdout
 *   - double-type power-normalized mel-cepstral coefficients
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);
  int impulse_response_length(kDefaultImpulseResponseLength);
  double alpha(kDefaultAlpha);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "m:l:a:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("pnorm", error_message);
          return 1;
        }
        break;
      }
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &impulse_response_length) ||
            impulse_response_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("mcpf", error_message);
          return 1;
        }
        break;
      }
      case 'a': {
        if (!sptk::ConvertStringToDouble(optarg, &alpha) ||
            !sptk::IsValidAlpha(alpha)) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -a option must be in (-1.0, 1.0)";
          sptk::PrintErrorMessage("pnorm", error_message);
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

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("pnorm", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  if (!sptk::SetBinaryMode()) {
    std::ostringstream error_message;
    error_message << "Cannot set translation mode";
    sptk::PrintErrorMessage("pnorm", error_message);
    return 1;
  }

  std::ifstream ifs;
  if (NULL != input_file) {
    ifs.open(input_file, std::ios::in | std::ios::binary);
    if (ifs.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << input_file;
      sptk::PrintErrorMessage("pnorm", error_message);
      return 1;
    }
  }
  std::istream& input_stream(ifs.is_open() ? ifs : std::cin);

  sptk::MelCepstrumPowerNormalization mel_cepstrum_power_normalization(
      num_order, impulse_response_length, alpha);
  sptk::MelCepstrumPowerNormalization::Buffer buffer;
  if (!mel_cepstrum_power_normalization.IsValid()) {
    std::ostringstream error_message;
    error_message << "FFT length must be a power of 2 and greater than 1";
    sptk::PrintErrorMessage("pnorm", error_message);
    return 1;
  }

  const int length(num_order + 1);
  std::vector<double> mel_cepstrum(length);
  double power;

  while (sptk::ReadStream(false, 0, 0, length, &mel_cepstrum, &input_stream,
                          NULL)) {
    if (!mel_cepstrum_power_normalization.Run(&mel_cepstrum, &power, &buffer)) {
      std::ostringstream error_message;
      error_message << "Failed to normalize mel-cepstrum";
      sptk::PrintErrorMessage("pnorm", error_message);
      return 1;
    }

    if (!sptk::WriteStream(power, &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write power";
      sptk::PrintErrorMessage("pnorm", error_message);
      return 1;
    }

    if (!sptk::WriteStream(0, length, mel_cepstrum, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write power-normalized mel-cepstrum";
      sptk::PrintErrorMessage("pnorm", error_message);
      return 1;
    }
  }

  return 0;
}
