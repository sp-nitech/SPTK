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

#include <algorithm>  // std::copy, std::reverse, std::transform
#include <complex>    // std::abs, std::arg, std::complex
#include <fstream>    // std::ifstream
#include <iomanip>    // std::setw
#include <iostream>   // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>    // std::ostringstream
#include <vector>     // std::vector

#include "Getopt/getoptwin.h"
#include "SPTK/math/durand_kerner_method.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum InputFormats { kForwardOrder = 0, kReverseOrder, kNumInputFormats };
enum OutputFormats { kRectangular = 0, kPolar, kNumOutputFormats };

const int kDefaultNumOrder(32);
const int kDefaultNumIteration(1000);
const double kDefaultConvergenceThreshold(1.0e-14);
const InputFormats kDefaultInputFormat(kForwardOrder);
const OutputFormats kDefaultOutputFormat(kRectangular);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " root_pol - find roots of polynomial" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       root_pol [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -m m  : order of polynomial          (   int)[" << std::setw(5) << std::right << kDefaultNumOrder             << "][   1 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -i i  : maximum number of iterations (   int)[" << std::setw(5) << std::right << kDefaultNumIteration         << "][   1 <= i <=   ]" << std::endl;  // NOLINT
  *stream << "       -d d  : convergence threshold        (double)[" << std::setw(5) << std::right << kDefaultConvergenceThreshold << "][ 0.0 <= d <=   ]" << std::endl;  // NOLINT
  *stream << "       -q q  : input format                 (   int)[" << std::setw(5) << std::right << kDefaultInputFormat          << "][   0 <= q <= 1 ]" << std::endl;  // NOLINT
  *stream << "                 0 (forward order)" << std::endl;
  *stream << "                 1 (reverse order)" << std::endl;
  *stream << "       -o o  : output format                (   int)[" << std::setw(5) << std::right << kDefaultOutputFormat         << "][   0 <= o <= 1 ]" << std::endl;  // NOLINT
  *stream << "                 0 (rectangular form)" << std::endl;
  *stream << "                 1 (polar form)" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       coefficients of polynomial           (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       roots of polynomial                  (double)" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a root_pol [ @e option ] [ @e infile ]
 *
 * - @b -m @e int
 *   - order of polynomial @f$(1 \le M)@f$
 * - @b -i @e int
 *   - maximum number of iterations
 * - @b -d @e double
 *   - convergence threshold
 * - @b -q @e int
 *   - input format
 *     \arg @c 0 forward order
 *     \arg @c 1 reverse order
 * - @b -o @e int
 *   - output format
 *     \arg @c 0 rectangular form
 *     \arg @c 1 polar form
 * - @b infile @e str
 *   - double-type coefficients of polynomial
 * - @b stdout
 *   - double-type roots of polynomial
 *
 * If @c -o is 0, real and imaginary parts of roots are written.
 *
 * @code{.sh}
 *   echo 3 4 5 | root_pol -m 2 -o 0 | x2x +da -c 2
 *   # -0.666667 1.10554
 *   # -0.666667 -1.10554
 * @endcode
 *
 * If @c -o is 1, radius and angle of roots are written.
 *
 * @code{.sh}
 *   echo 3 4 5 | root_pol -m 2 -o 1 | x2x +da -c 2
 *   # 1.29099 2.11344
 *   # 1.29099 -2.11344
 * @endcode
 */
int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);
  int num_iteration(kDefaultNumIteration);
  double convergence_threshold(kDefaultConvergenceThreshold);
  InputFormats input_format(kDefaultInputFormat);
  OutputFormats output_format(kDefaultOutputFormat);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "m:i:d:q:o:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -m option must be a positive integer";
          sptk::PrintErrorMessage("root_pol", error_message);
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
          sptk::PrintErrorMessage("root_pol", error_message);
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
          sptk::PrintErrorMessage("root_pol", error_message);
          return 1;
        }
        break;
      }
      case 'q': {
        const int min(0);
        const int max(static_cast<int>(kNumInputFormats) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -q option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("root_pol", error_message);
          return 1;
        }
        input_format = static_cast<InputFormats>(tmp);
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
          sptk::PrintErrorMessage("root_pol", error_message);
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

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("root_pol", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("root_pol", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::DurandKernerMethod durand_kerner_method(num_order, num_iteration,
                                                convergence_threshold);
  if (!durand_kerner_method.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize DurandKernerMethod";
    sptk::PrintErrorMessage("root_pol", error_message);
    return 1;
  }

  std::vector<double> coefficients(num_order + 1);
  std::vector<double> normalized_coefficients(num_order);
  std::vector<std::complex<double> > roots(num_order);

  if (sptk::ReadStream(false, 0, 0, num_order + 1, &coefficients, &input_stream,
                       NULL)) {
    switch (input_format) {
      case kForwardOrder: {
        // nothing to do
        break;
      }
      case kReverseOrder: {
        std::reverse(coefficients.begin(), coefficients.end());
        break;
      }
      default: {
        break;
      }
    }

    if (0.0 == coefficients[0]) {
      std::ostringstream error_message;
      error_message << "Leading coefficient must not be zero";
      sptk::PrintErrorMessage("root_pol", error_message);
      return 1;
    } else if (1.0 == coefficients[0]) {
      std::copy(coefficients.begin() + 1, coefficients.end(),
                normalized_coefficients.begin());
    } else {
      const double z(1.0 / coefficients[0]);
      std::transform(coefficients.begin() + 1, coefficients.end(),
                     normalized_coefficients.begin(),
                     [z](double x) { return x * z; });
    }

    bool is_converged;
    if (!durand_kerner_method.Run(normalized_coefficients, &roots,
                                  &is_converged)) {
      std::ostringstream error_message;
      error_message << "Failed to run Durand-Kerner method";
      sptk::PrintErrorMessage("root_pol", error_message);
      return 1;
    }

    if (!is_converged) {
      std::ostringstream error_message;
      error_message << "Could not reach convergence";
      sptk::PrintErrorMessage("root_pol", error_message);
      return 1;
    }

    switch (output_format) {
      case kRectangular: {
        for (int m(0); m < num_order; ++m) {
          if (!sptk::WriteStream(roots[m].real(), &std::cout)) {
            std::ostringstream error_message;
            error_message << "Failed to write real part";
            sptk::PrintErrorMessage("root_pol", error_message);
            return 1;
          }
          if (!sptk::WriteStream(roots[m].imag(), &std::cout)) {
            std::ostringstream error_message;
            error_message << "Failed to write imaginary part";
            sptk::PrintErrorMessage("root_pol", error_message);
            return 1;
          }
        }
        break;
      }
      case kPolar: {
        for (int m(0); m < num_order; ++m) {
          if (!sptk::WriteStream(std::abs(roots[m]), &std::cout)) {
            std::ostringstream error_message;
            error_message << "Failed to write radius";
            sptk::PrintErrorMessage("root_pol", error_message);
            return 1;
          }
          if (!sptk::WriteStream(std::arg(roots[m]), &std::cout)) {
            std::ostringstream error_message;
            error_message << "Failed to write angle";
            sptk::PrintErrorMessage("root_pol", error_message);
            return 1;
          }
        }
        break;
      }
      default: {
        break;
      }
    }
  }

  return 0;
}
