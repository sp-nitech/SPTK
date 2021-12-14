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
#include "SPTK/conversion/mlsa_digital_filter_coefficients_to_mel_cepstrum.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultNumOrder(25);
const double kDefaultAlpha(0.35);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " b2mc - convert MLSA digital filter coefficients to mel-cepstrum" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       b2mc [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -m m  : order of mel-cepstrum (   int)[" << std::setw(5) << std::right << kDefaultNumOrder << "][    0 <= m <=     ]" << std::endl;  // NOLINT
  *stream << "       -a a  : all-pass constant     (double)[" << std::setw(5) << std::right << kDefaultAlpha    << "][ -1.0 <  a <  1.0 ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       MLSA filter coefficients      (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       mel-cepstrum                  (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a b2mc [ @e option ] [ @e infile ]
 *
 * - @b -m @e int
 *   - order of coefficients @f$(0 \le M)@f$
 * - @b -a @e double
 *   - all-pass constant @f$(|\alpha| < 1)@f$
 * - @b infile @e str
 *   - double-type MLSA digital filter coefficients
 * - @b stdout
 *   - double-type mel-cepstral coefficients
 *
 * The below example converts MLSA digital filter coefficients into mel-cepstral
 * coefficients:
 *
 * @code{.sh}
 *   b2mc < data.b > data.mc
 * @endcode
 *
 * The converted mel-cepstral coefficients can be reverted by
 *
 * @code{.sh}
 *   mc2b < data.mc > data.b
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);
  double alpha(kDefaultAlpha);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "m:a:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("b2mc", error_message);
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
          sptk::PrintErrorMessage("b2mc", error_message);
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
    sptk::PrintErrorMessage("b2mc", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("b2mc", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::MlsaDigitalFilterCoefficientsToMelCepstrum
      mlsa_digital_filter_coefficients_to_mel_cepstrum(num_order, alpha);
  if (!mlsa_digital_filter_coefficients_to_mel_cepstrum.IsValid()) {
    std::ostringstream error_message;
    error_message
        << "Failed to initialize MlsaDigitalFilterCoefficientsToMelCepstrum";
    sptk::PrintErrorMessage("b2mc", error_message);
    return 1;
  }

  const int length(num_order + 1);
  std::vector<double> mlsa_digital_filter_coefficients(length);
  std::vector<double> mel_cepstrum(length);

  while (sptk::ReadStream(false, 0, 0, length,
                          &mlsa_digital_filter_coefficients, &input_stream,
                          NULL)) {
    if (!mlsa_digital_filter_coefficients_to_mel_cepstrum.Run(
            mlsa_digital_filter_coefficients, &mel_cepstrum)) {
      std::ostringstream error_message;
      error_message << "Failed to convert MLSA digital filter coefficients to "
                    << "mel-cepstrum";
      sptk::PrintErrorMessage("b2mc", error_message);
      return 1;
    }

    if (!sptk::WriteStream(0, length, mel_cepstrum, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write mel-cepstrum";
      sptk::PrintErrorMessage("b2mc", error_message);
      return 1;
    }
  }

  return 0;
}
