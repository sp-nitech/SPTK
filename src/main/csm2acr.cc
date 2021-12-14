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
#include "SPTK/conversion/composite_sinusoidal_modeling_to_autocorrelation.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultNumOrder(25);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " csm2acr - convert composite sinusoidal modeling to autocorrelation" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       csm2acr [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -m m  : order of autocorrelation (   int)[" << std::setw(5) << std::right << kDefaultNumOrder << "][ 1 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       composite sinusoidal modeling    (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       autocorrelation                  (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a csm2acr [ @e option ] [ @e infile ]
 *
 * - @b -m @e int
 *   - order of autocorrelation @f$(1 \le M)@f$
 * - @b infile @e str
 *   - double-type CSM parameters
 * - @b stdout
 *   - double-type autocorrelation
 *
 * The below example converts CSM parameters into autocorrelation coefficients:
 *
 * @code{.sh}
 *   csm2acr < data.csm > data.acr
 * @endcode
 *
 * The converted autocorrelation coefficients can be reverted by
 *
 * @code{.sh}
 *   acr2csm < data.acr > data.csm
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);

  for (;;) {
    const char option_char(getopt_long(argc, argv, "m:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order <= 0 || 0 == num_order % 2) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a positive "
                           "odd integer";
          sptk::PrintErrorMessage("csm2acr", error_message);
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
    sptk::PrintErrorMessage("csm2acr", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("csm2acr", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  const int length(num_order + 1);
  const int num_sine_wave(length / 2);
  sptk::CompositeSinusoidalModelingToAutocorrelation
      composite_sinusoidal_modeling_to_autocorrelation(num_sine_wave);
  if (!composite_sinusoidal_modeling_to_autocorrelation.IsValid()) {
    std::ostringstream error_message;
    error_message
        << "Failed to initialize CompositeSinusoidalModelingToAutocorrelation";
    sptk::PrintErrorMessage("csm2acr", error_message);
    return 1;
  }

  std::vector<double> composite_sinusoidal_modeling(length);
  std::vector<double> autocorrelation(length);

  while (sptk::ReadStream(false, 0, 0, length, &composite_sinusoidal_modeling,
                          &input_stream, NULL)) {
    if (!composite_sinusoidal_modeling_to_autocorrelation.Run(
            composite_sinusoidal_modeling, &autocorrelation)) {
      std::ostringstream error_message;
      error_message << "Failed to convert composite sinusoidal modeling to "
                    << "autocorrelation";
      sptk::PrintErrorMessage("csm2acr", error_message);
      return 1;
    }

    if (!sptk::WriteStream(0, length, autocorrelation, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write autocorrelation";
      sptk::PrintErrorMessage("csm2acr", error_message);
      return 1;
    }
  }

  return 0;
}
