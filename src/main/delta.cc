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
//                1996-2020  Nagoya Institute of Technology          //
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

#include <getopt.h>  // getopt_long

#include <fstream>   // std::ifstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream
#include <vector>    // std::vector

#include "SPTK/generation/delta_calculation.h"
#include "SPTK/input/input_source_from_stream.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum LongOptions {
  kMagic = 1000,
};

const int kDefaultNumOrder(24);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " delta - delta calculation" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       delta [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l          : length of vector        (   int)[" << std::setw(5) << std::right << kDefaultNumOrder + 1 << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m          : order of vector         (   int)[" << std::setw(5) << std::right << "l-1"                << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -d d1 d2 ...  : delta coefficients      (double)[" << std::setw(5) << std::right << "N/A"                << "]" << std::endl;  // NOLINT
  *stream << "       -D D          : filename of double type (string)[" << std::setw(5) << std::right << "N/A"                << "]" << std::endl;  // NOLINT
  *stream << "                       delta coefficients" << std::endl;
  *stream << "       -r r1 (r2)    : width of regression     (   int)[" << std::setw(5) << std::right << "N/A"                << "]" << std::endl;  // NOLINT
  *stream << "                       coefficients" << std::endl;
  *stream << "       -magic magic  : magic number            (double)[" << std::setw(5) << std::right << "N/A"                << "]" << std::endl;  // NOLINT
  *stream << "       -h            : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       static feature vectors                  (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       static and dynamic feature vectors      (double)" << std::endl;  // NOLINT
  *stream << "  notice:" << std::endl;
  *stream << "       -d and -D options can be given multiple times" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a delta [ @e option ] [ @e infile ]
 *
 * - @b -l @e int
 *   - length of vector @f$(1 \le M + 1)@f$
 * - @b -m @e int
 *   - order of vector @f$(0 \le M)@f$
 * - @b -d @e double+
 *   - delta coefficients
 * - @b -D @e string
 *   - filename of double-type delta coefficients
 * - @b -r @e int+
 *   - width of 1st (and 2nd) regression coefficients
 * - @b -magic @e double
 *   - magic number
 * - @b infile @e str
 *   - double-type static feature vectors
 * - @b stdout
 *   - double-type static and dynamic feature vectors
 *
 * The below examples calculate the first and second order dynamic features from
 * 15-dimensional coefficient vectors in @c data.d.
 *
 * @code{.sh}
 *   delta -l 15 -d -0.5 0.0 0.5 -d 1.0 -2.0 1.0 < data.d > data.delta
 * @endcode
 *
 * This is equivalent to
 *
 * @code{.sh}
 *   echo -0.5 0.0 0.5 | x2x +ad > delta.win
 *   echo 1.0 2.0 -1.0 | x2x +ad > accel.win
 *   delta -l 15 -D delta.win -D accel.win < data.d > data.delta
 * @endcode
 *
 * If data contains a special number such as an unvoiced symbol in a sequence
 * of fundamental frequencies, use @b -magic option:
 * @code{.sh}
 *   delta -l 15 -D delta.win -magic -1e+10 < data.lf0 > data.lf0.delta
 * @endcode
 *
 * @b -r option specifies the width of regression coefficients, @f$L^{(1)}@f$
 * and @f$L^{(2)}@f$. The first and second derivatives are then calculated as
 * follows:
 * @f{eqnarray}{
 *   \Delta^{(1)} x_t &=&
 *     \frac{\displaystyle\sum_{\tau=-L^{(1)}}^{L^{(1)}} \tau \, x_{t+\tau}}
 *          {\displaystyle\sum_{\tau=-L^{(1)}}^{L^{(1)}} \tau^2}, \\
 *   \Delta^{(2)} x_t &=&
 *     \frac{\displaystyle\sum_{\tau=-L^{(2)}}^{L^{(2)}}
 *           2 (a_0 \tau^2 - a_1) x_{t+\tau}}
 *          {a_2 a_0 - a_1^2},
 * @f}
 * where
 * @f{eqnarray}{
 *   a_0 &=& \displaystyle\sum_{\tau=-L^{(2)}}^{L^{(2)}} 1, \\
 *   a_1 &=& \displaystyle\sum_{\tau=-L^{(2)}}^{L^{(2)}} \tau^2, \\
 *   a_2 &=& \displaystyle\sum_{\tau=-L^{(2)}}^{L^{(2)}} \tau^4.
 * @f}
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);
  std::vector<std::vector<double> > window_coefficients({{1.0}});
  bool is_regression_specified(false);
  double magic_number(0.0);
  bool is_magic_number_specified(false);

  const struct option long_options[] = {
      {"magic", required_argument, NULL, kMagic}, {0, 0, 0, 0},
  };

  for (;;) {
    const int option_char(
        getopt_long_only(argc, argv, "l:m:d:D:r:h", long_options, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("delta", error_message);
          return 1;
        }
        --num_order;
        break;
      }
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("delta", error_message);
          return 1;
        }
        break;
      }
      case 'd': {
        if (is_regression_specified) {
          std::ostringstream error_message;
          error_message << "-d and -r options cannot be specified at the same";
          sptk::PrintErrorMessage("delta", error_message);
          return 1;
        }

        std::vector<double> coefficients;
        double coefficient;
        if (!sptk::ConvertStringToDouble(optarg, &coefficient)) {
          std::ostringstream error_message;
          error_message << "The argument for the -d option must be numeric";
          sptk::PrintErrorMessage("delta", error_message);
          return 1;
        }
        coefficients.push_back(coefficient);
        while (optind < argc &&
               sptk::ConvertStringToDouble(argv[optind], &coefficient)) {
          coefficients.push_back(coefficient);
          ++optind;
        }
        window_coefficients.push_back(coefficients);
        break;
      }
      case 'D': {
        if (is_regression_specified) {
          std::ostringstream error_message;
          error_message << "-D and -r options cannot be specified at the same";
          sptk::PrintErrorMessage("delta", error_message);
          return 1;
        }

        std::ifstream ifs;
        ifs.open(optarg, std::ios::in | std::ios::binary);
        if (ifs.fail()) {
          std::ostringstream error_message;
          error_message << "Cannot open file " << optarg;
          sptk::PrintErrorMessage("delta", error_message);
          return 1;
        }
        std::vector<double> coefficients;
        double coefficient;
        while (sptk::ReadStream(&coefficient, &ifs)) {
          coefficients.push_back(coefficient);
        }
        window_coefficients.push_back(coefficients);
        break;
      }
      case 'r': {
        if (is_regression_specified) {
          std::ostringstream error_message;
          error_message << "-r option cannot be specified multiple times";
          sptk::PrintErrorMessage("delta", error_message);
          return 1;
        }

        int n;
        // Set first order coefficients.
        {
          if (!sptk::ConvertStringToInteger(optarg, &n) || n <= 0) {
            std::ostringstream error_message;
            error_message
                << "The argument for the -r option must be positive integer(s)";
            sptk::PrintErrorMessage("delta", error_message);
            return 1;
          }

          std::vector<double> coefficients(2 * n + 1);
          const int a1(n * (n + 1) * (2 * n + 1) / 3);
          const double norm(1.0 / a1);
          for (int j(-n), i(0); j <= n; ++j, ++i) {
            coefficients[i] = j * norm;
          }
          window_coefficients.push_back(coefficients);
        }

        // Set second order coefficients.
        if (optind < argc && sptk::ConvertStringToInteger(argv[optind], &n)) {
          if (n <= 0) {
            std::ostringstream error_message;
            error_message
                << "The argument for the -r option must be positive integer(s)";
            sptk::PrintErrorMessage("delta", error_message);
            return 1;
          }
          std::vector<double> coefficients(2 * n + 1);
          const int a0(2 * n + 1);
          const int a1(a0 * n * (n + 1) / 3);
          const int a2(a1 * (3 * n * n + 3 * n - 1) / 5);
          const double norm(2.0 / (a2 * a0 - a1 * a1));
          for (int j(-n), i(0); j <= n; ++j, ++i) {
            coefficients[i] = (a0 * j * j - a1) * norm;
          }
          window_coefficients.push_back(coefficients);
          ++optind;
        }
        is_regression_specified = true;
        break;
      }
      case kMagic: {
        if (!sptk::ConvertStringToDouble(optarg, &magic_number)) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -magic option must be a number";
          sptk::PrintErrorMessage("delta", error_message);
          return 1;
        }
        is_magic_number_specified = true;
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
    sptk::PrintErrorMessage("delta", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("delta", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  const int input_length(num_order + 1);
  sptk::InputSourceFromStream input_source(false, input_length, &input_stream);
  sptk::DeltaCalculation delta_calculation(
      num_order, window_coefficients, &input_source, is_magic_number_specified,
      magic_number);
  if (!delta_calculation.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialze DeltaCalculation";
    sptk::PrintErrorMessage("delta", error_message);
    return 1;
  }

  const int output_length(input_length * window_coefficients.size());
  std::vector<double> output(output_length);

  while (delta_calculation.Get(&output)) {
    if (!sptk::WriteStream(0, output_length, output, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write delta";
      sptk::PrintErrorMessage("delta", error_message);
      return 1;
    }
  }

  return 0;
}
