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
#include "SPTK/math/reverse_levinson_durbin_recursion.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultNumOrder(25);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " rlevdur - solve autocorrelation normal equation by reverse Levinson-Durbin recursion" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       rlevdur [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -m m  : order of autocorrelation (   int)[" << std::setw(5) << std::right << kDefaultNumOrder << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       linear predictive coefficients   (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       autocorrelation                  (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a rlevdur [ @e option ] [ @e infile ]
 *
 * - @b -m @e int
 *   - order of coefficients @f$(0 \le M)@f$
 * - @b infile @e str
 *   - double-type linear predictive coefficients
 * - @b stdout
 *   - double-type autocorrelation
 *
 * The below example converts LPC coefficients in @c data.lpc to CSM parameters.
 *
 * @code{.sh}
 *   rlevdur -m 10 < data.lpc | acr2csm -m 10 > data.csm
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
          sptk::PrintErrorMessage("rlevdur", error_message);
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
    sptk::PrintErrorMessage("rlevdur", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("rlevdur", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::ReverseLevinsonDurbinRecursion reverse_levinson_durbin_recursion(
      num_order);
  sptk::ReverseLevinsonDurbinRecursion::Buffer buffer;
  if (!reverse_levinson_durbin_recursion.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize ReverseLevinsonDurbinRecursion";
    sptk::PrintErrorMessage("rlevdur", error_message);
    return 1;
  }

  const int length(num_order + 1);
  std::vector<double> autocorrelation(length);
  std::vector<double> linear_predictive_coefficients(length);

  while (sptk::ReadStream(false, 0, 0, length, &linear_predictive_coefficients,
                          &input_stream, NULL)) {
    if (!reverse_levinson_durbin_recursion.Run(linear_predictive_coefficients,
                                               &autocorrelation, &buffer)) {
      std::ostringstream error_message;
      error_message << "Failed to solve autocorrelation normal equations";
      sptk::PrintErrorMessage("rlevdur", error_message);
      return 1;
    }

    if (!sptk::WriteStream(0, length, autocorrelation, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write autocorrelation";
      sptk::PrintErrorMessage("rlevdur", error_message);
      return 1;
    }
  }

  return 0;
}
