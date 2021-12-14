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
#include "SPTK/conversion/parcor_coefficients_to_linear_predictive_coefficients.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultNumOrder(25);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " par2lpc - convert PARCOR coefficients to linear predictive coefficients" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       par2lpc [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -m m  : order of coefficients  (   int)[" << std::setw(5) << std::right << kDefaultNumOrder << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       PARCOR coefficients            (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       linear predictive coefficients (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a par2lpc [ @e option ] [ @e infile ]
 *
 * - @b -m @e int
 *   - order of coefficients @f$(0 \le M)@f$
 * - @b infile @e str
 *   - double-type PARCOR coefficients
 * - @b stdout
 *   - double-type LPC coefficients
 *
 * The below example converts PARCOR coefficients into LPC coefficients:
 *
 * @code{.sh}
 *   par2lpc < data.rc > data.lpc
 * @endcode
 *
 * The converted LPC coefficients can be reverted by
 *
 * @code{.sh}
 *   lpc2par < data.lpc > data.rc
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "m:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("par2lpc", error_message);
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
    sptk::PrintErrorMessage("par2lpc", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("par2lpc", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::ParcorCoefficientsToLinearPredictiveCoefficients
      parcor_coefficients_to_linear_predictive_coefficients(num_order);
  sptk::ParcorCoefficientsToLinearPredictiveCoefficients::Buffer buffer;
  if (!parcor_coefficients_to_linear_predictive_coefficients.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize "
                  << "ParcorCoefficientsToLinearPredictiveCoefficients";
    sptk::PrintErrorMessage("par2lpc", error_message);
    return 1;
  }

  const int length(num_order + 1);
  std::vector<double> coefficients(length);

  while (sptk::ReadStream(false, 0, 0, length, &coefficients, &input_stream,
                          NULL)) {
    if (!parcor_coefficients_to_linear_predictive_coefficients.Run(
            &coefficients, &buffer)) {
      std::ostringstream error_message;
      error_message << "Failed to convert PARCOR coefficients to "
                    << "linear predictive coefficients";
      sptk::PrintErrorMessage("par2lpc", error_message);
      return 1;
    }

    if (!sptk::WriteStream(0, length, coefficients, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write linear predictive coefficients";
      sptk::PrintErrorMessage("par2lpc", error_message);
      return 1;
    }
  }

  return 0;
}
