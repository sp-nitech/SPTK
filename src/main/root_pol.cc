// ----------------------------------------------------------------- //
//             The Speech Signal Processing Toolkit (SPTK)           //
//             developed by SPTK Working Group                       //
//             http://sp-tk.sourceforge.net/                         //
// ----------------------------------------------------------------- //
//                                                                   //
//  Copyright (c) 1984-2007  Tokyo Institute of Technology           //
//                           Interdisciplinary Graduate School of    //
//                           Science and Engineering                 //
//                                                                   //
//                1996-2017  Nagoya Institute of Technology          //
//                           Department of Computer Science          //
//                                                                   //
// All rights reserved.                                              //
//                                                                   //
// Redistribution and use in source and binary forms, with or        //
// without modification, are permitted provided that the following   //
// conditions are met:                                               //
//                                                                   //
// - Redistributions of source code must retain the above copyright  //
//   notice, this list of conditions and the following disclaimer.   //
// - Redistributions in binary form must reproduce the above         //
//   copyright notice, this list of conditions and the following     //
//   disclaimer in the documentation and/or other materials provided //
//   with the distribution.                                          //
// - Neither the name of the SPTK working group nor the names of its //
//   contributors may be used to endorse or promote products derived //
//   from this software without specific prior written permission.   //
//                                                                   //
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            //
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       //
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          //
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          //
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS //
// BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          //
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   //
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     //
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON //
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   //
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    //
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           //
// POSSIBILITY OF SUCH DAMAGE.                                       //
// ----------------------------------------------------------------- //

#include <getopt.h>
#include <algorithm>
#include <complex>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

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
  *stream << "       -m m  : order of polynomial         (   int)[" << std::setw(5) << std::right << kDefaultNumOrder             << "][   1 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -i i  : maximum number of iteration (   int)[" << std::setw(5) << std::right << kDefaultNumIteration         << "][   1 <= i <=   ]" << std::endl;  // NOLINT
  *stream << "       -d d  : end condition of iteration  (double)[" << std::setw(5) << std::right << kDefaultConvergenceThreshold << "][ 0.0 <= d <=   ]" << std::endl;  // NOLINT
  *stream << "       -q q  : input format                (   int)[" << std::setw(5) << std::right << kDefaultInputFormat          << "][   0 <= q <= 1 ]" << std::endl;  // NOLINT
  *stream << "                 0 (forward order)" << std::endl;
  *stream << "                 1 (reverse order)" << std::endl;
  *stream << "       -o o  : output format               (   int)[" << std::setw(5) << std::right << kDefaultOutputFormat         << "][   0 <= o <= 1 ]" << std::endl;  // NOLINT
  *stream << "                 0 (rectangular form)" << std::endl;
  *stream << "                 1 (polar form)" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       coefficients of polynomial          (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       roots of polynomial                 (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

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

  // get input file
  const int num_rest_args(argc - optind);
  if (1 < num_rest_args) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("root_pol", error_message);
    return 1;
  }
  const char* input_file(0 == num_rest_args ? NULL : argv[optind]);

  // open stream
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
    error_message << "Failed to set condition for finding roots";
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
      default: { break; }
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
      std::transform(
          coefficients.begin() + 1, coefficients.end(),
          normalized_coefficients.begin(),
          std::bind1st(std::multiplies<double>(), 1.0 / coefficients[0]));
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
      error_message << "No convergence";
      sptk::PrintErrorMessage("root_pol", error_message);
      return 1;
    }

    switch (output_format) {
      case kRectangular: {
        for (int i(0); i < num_order; ++i) {
          if (!sptk::WriteStream(roots[i].real(), &std::cout)) {
            std::ostringstream error_message;
            error_message << "Failed to write real part";
            sptk::PrintErrorMessage("root_pol", error_message);
            return 1;
          }
          if (!sptk::WriteStream(roots[i].imag(), &std::cout)) {
            std::ostringstream error_message;
            error_message << "Failed to write imaginary part";
            sptk::PrintErrorMessage("root_pol", error_message);
            return 1;
          }
        }
        break;
      }
      case kPolar: {
        for (int i(0); i < num_order; ++i) {
          if (!sptk::WriteStream(std::abs(roots[i]), &std::cout)) {
            std::ostringstream error_message;
            error_message << "Failed to write radius";
            sptk::PrintErrorMessage("root_pol", error_message);
            return 1;
          }
          if (!sptk::WriteStream(std::arg(roots[i]), &std::cout)) {
            std::ostringstream error_message;
            error_message << "Failed to write angle";
            sptk::PrintErrorMessage("root_pol", error_message);
            return 1;
          }
        }
        break;
      }
      default: { break; }
    }
  }

  return 0;
}
