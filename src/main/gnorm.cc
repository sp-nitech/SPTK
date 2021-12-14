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
#include "SPTK/conversion/generalized_cepstrum_gain_normalization.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultNumOrder(25);
const double kDefaultGamma(0.0);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " gnorm - gain normalization of generalized cepstrum" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       gnorm [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -m m  : order of generalized cepstrum (   int)[" << std::setw(5) << std::right << kDefaultNumOrder << "][    0 <= m <=     ]" << std::endl;  // NOLINT
  *stream << "       -g g  : gamma                         (double)[" << std::setw(5) << std::right << kDefaultGamma    << "][ -1.0 <= g <= 1.0 ]" << std::endl;  // NOLINT
  *stream << "       -c c  : gamma = -1 / c                (   int)[" << std::setw(5) << std::right << "N/A"            << "][    1 <= c <=     ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       generalized cepstrum                  (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       normalized generalized cepstrum       (double)" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a gnorm [ @e option ] [ @e infile ]
 *
 * - @b -m @e int
 *   - order of coefficients @f$(0 \le M)@f$
 * - @b -g @e double
 *   - gamma @f$(|\gamma| \le 1)@f$
 * - @b -c @e int
 *   - gamma @f$\gamma = -1 / C@f$ @f$(1 \le C)@f$
 * - @b infile @e str
 *   - double-type generalized cepstral coefficients
 * - @b stdout
 *   - double-type normalized generalized cepstral coefficients
 *
 * The below example normalizes generalized cepstral coefficients:
 *
 * @code{.sh}
 *   gnorm -g -0.5 < data.gc > data.ngc
 * @endcode
 *
 * The normalized generalized cepstral coefficients can be reverted by
 *
 * @code{.sh}
 *   ignorm -g -0.5 < data.ngc > data.gc
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);
  double gamma(kDefaultGamma);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "m:g:c:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("gnorm", error_message);
          return 1;
        }
        break;
      }
      case 'g': {
        if (!sptk::ConvertStringToDouble(optarg, &gamma) ||
            !sptk::IsValidGamma(gamma)) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -g option must be in [-1.0, 1.0]";
          sptk::PrintErrorMessage("gnorm", error_message);
          return 1;
        }
        break;
      }
      case 'c': {
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) || tmp < 1) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -c option must be a positive integer";
          sptk::PrintErrorMessage("gnorm", error_message);
          return 1;
        }
        gamma = -1.0 / tmp;
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
    sptk::PrintErrorMessage("gnorm", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("gnorm", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::GeneralizedCepstrumGainNormalization
      generalized_cepstrum_gain_normalization(num_order, gamma);
  if (!generalized_cepstrum_gain_normalization.IsValid()) {
    std::ostringstream error_message;
    error_message
        << "Failed to initialize GeneralizedCepstrumGainNormalization";
    sptk::PrintErrorMessage("gnorm", error_message);
    return 1;
  }

  const int length(num_order + 1);
  std::vector<double> generalized_cepstrum(length);

  while (sptk::ReadStream(false, 0, 0, length, &generalized_cepstrum,
                          &input_stream, NULL)) {
    if (!generalized_cepstrum_gain_normalization.Run(&generalized_cepstrum)) {
      std::ostringstream error_message;
      error_message << "Failed to normalize generalized cepstrum";
      sptk::PrintErrorMessage("gnorm", error_message);
      return 1;
    }

    if (!sptk::WriteStream(0, length, generalized_cepstrum, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write normalized generalized cepstrum";
      sptk::PrintErrorMessage("gnorm", error_message);
      return 1;
    }
  }

  return 0;
}
