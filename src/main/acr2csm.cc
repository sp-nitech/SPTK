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
#include "SPTK/conversion/autocorrelation_to_composite_sinusoidal_modeling.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultNumOrder(25);
const int kDefaultNumIteration(1000);
const double kDefaultConvergenceThreshold(1e-12);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " acr2csm - convert autocorrelation to composite sinusoidal modeling (CSM)" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       acr2csm [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -m m  : order of autocorrelation     (   int)[" << std::setw(5) << std::right << kDefaultNumOrder             << "][   1 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -i i  : maximum number of iterations (   int)[" << std::setw(5) << std::right << kDefaultNumIteration         << "][   1 <= i <=   ]" << std::endl;  // NOLINT
  *stream << "       -d d  : convergence threshold        (double)[" << std::setw(5) << std::right << kDefaultConvergenceThreshold << "][ 0.0 <= d <=   ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       autocorrelation                      (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       composite sinusoidal modeling        (double)" << std::endl;  // NOLINT
  *stream << "  notice:" << std::endl;
  *stream << "       value of m must be odd" << std::endl;
  *stream << "       if m > 30, cannot compute reliable CSM due to computational accuracy" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a acr2csm [ @e option ] [ @e infile ]
 *
 * - @b -m @e int
 *   - order of autocorrelation @f$(1 \le M)@f$
 * - @b -i @e int
 *   - number of iterations of root finding @f$(1 \le N)@f$
 * - @b -d @e double
 *   - convergence threshold of root finding @f$(0 \le \epsilon)@f$
 * - @b infile @e str
 *   - double-type autocorrelation
 * - @b stdout
 *   - double-type CSM parameters
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);
  int num_iteration(kDefaultNumIteration);
  double convergence_threshold(kDefaultConvergenceThreshold);

  for (;;) {
    const char option_char(getopt_long(argc, argv, "m:i:d:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order <= 0 || 0 == num_order % 2) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                           "positive odd integer";
          sptk::PrintErrorMessage("acr2csm", error_message);
          return 1;
        }
        break;
      }
      case 'i': {
        if (!sptk::ConvertStringToInteger(optarg, &num_iteration) ||
            num_iteration <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -i option must be a positive integer";
          sptk::PrintErrorMessage("acr2csm", error_message);
          return 1;
        }
        break;
      }
      case 'd': {
        if (!sptk::ConvertStringToDouble(optarg, &convergence_threshold) ||
            convergence_threshold < 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -d option must be a non-negative number";
          sptk::PrintErrorMessage("acr2csm", error_message);
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
    sptk::PrintErrorMessage("acr2csm", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("acr2csm", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::AutocorrelationToCompositeSinusoidalModeling
      autocorrelation_to_composite_sinusoidal_modeling(num_order, num_iteration,
                                                       convergence_threshold);
  sptk::AutocorrelationToCompositeSinusoidalModeling::Buffer buffer;
  if (!autocorrelation_to_composite_sinusoidal_modeling.IsValid()) {
    std::ostringstream error_message;
    error_message
        << "Failed to initialize AutocorrelationToCompositeSinusoidalModeling";
    sptk::PrintErrorMessage("acr2csm", error_message);
    return 1;
  }

  const int length(num_order + 1);
  std::vector<double> autocorrelation(length);
  std::vector<double> composite_sinusoidal_modeling(length);

  while (sptk::ReadStream(false, 0, 0, length, &autocorrelation, &input_stream,
                          NULL)) {
    if (!autocorrelation_to_composite_sinusoidal_modeling.Run(
            autocorrelation, &composite_sinusoidal_modeling, &buffer)) {
      std::ostringstream error_message;
      error_message << "Failed to convert autocorrelation to composite "
                       "sinusoidal modeling coefficients";
      sptk::PrintErrorMessage("acr2csm", error_message);
      return 1;
    }

    if (!sptk::WriteStream(0, length, composite_sinusoidal_modeling, &std::cout,
                           NULL)) {
      std::ostringstream error_message;
      error_message
          << "Failed to write composite sinusoidal modeling coefficients";
      sptk::PrintErrorMessage("acr2csm", error_message);
      return 1;
    }
  }

  return 0;
}
