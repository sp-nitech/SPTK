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

#include <cmath>     // std::fabs
#include <fstream>   // std::ifstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream

#include "SPTK/utils/sptk_utils.h"

namespace {

const double kDefaultTolerance(1e-6);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " aeq - check almost equality" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       aeq [ options ] exfile [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -t t  : tolerance          (double)[" << std::setw(5) << std::right << kDefaultTolerance << "][ 0.0 <= t <=   ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  exfile:" << std::endl;
  *stream << "       expected values            (double)" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       actual values              (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       result messages" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * \a aeq [ \e option ] \e exfile [ \e infile ]
 *
 * - \b -t \e double
 *   - absolute tolerance \f$(0 \le \epsilon)\f$
 * - \b exfile \e str
 *   - double-type expected values
 * - \b infile \e str
 *   - double-type actual values
 * - \b stdout
 *   - result messages
 *
 * This command checks whether two data sequences are almost equal or not.
 *
 * Given the two data sequences
 * \f[
 *   \begin{array}{cccc}
 *     x_1(0), & x_1(1), & \ldots, & x_1(T_1), \\
 *     x_2(0), & x_2(1), & \ldots, & x_2(T_2), \\
 *   \end{array}
 * \f]
 * the absolute error \f$e\f$ is calculated sample-by-sample:
 * \f[
 *   e(t) = | x_1(t) - x_2(t) |.
 * \f]
 * If \f$e(t)\f$ is greater than tolerance \f$\epsilon\f$ or \f$T_1\f$ is not
 * equal to \f$T_2\f$, the command prints a warning message.
 *
 * The below example checks the equality between two data:
 *
 * @code{.sh}
 *   aeq -t 0 data.x1 data.x2
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @retval -1 Given two data sequences are not almost equal.
 * @retval 0 Given two data sequences are almost equal.
 * @retval 1 Failed to run this command.
 */
int main(int argc, char* argv[]) {
  double tolerance(kDefaultTolerance);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "t:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 't': {
        if (!sptk::ConvertStringToDouble(optarg, &tolerance) ||
            tolerance < 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -t option must be a non-negative number";
          sptk::PrintErrorMessage("aeq", error_message);
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

  const char* expected_file;
  const char* actual_file;
  const int num_input_files(argc - optind);
  if (2 == num_input_files) {
    expected_file = argv[argc - 2];
    actual_file = argv[argc - 1];
  } else if (1 == num_input_files) {
    expected_file = argv[argc - 1];
    actual_file = NULL;
  } else {
    std::ostringstream error_message;
    error_message << "Just two input files, exfile and infile, are required";
    sptk::PrintErrorMessage("aeq", error_message);
    return 1;
  }

  std::ifstream ifs1;
  ifs1.open(expected_file, std::ios::in | std::ios::binary);
  if (ifs1.fail()) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << expected_file;
    sptk::PrintErrorMessage("aeq", error_message);
    return 1;
  }
  std::istream& stream_for_expected(ifs1);

  std::ifstream ifs2;
  ifs2.open(actual_file, std::ios::in | std::ios::binary);
  if (ifs2.fail() && NULL != actual_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << actual_file;
    sptk::PrintErrorMessage("aeq", error_message);
    return 1;
  }
  std::istream& stream_for_actual(ifs2.fail() ? std::cin : ifs2);

  double expected;
  double actual;
  int status(0);

  for (int sample_index(0);; ++sample_index) {
    const bool result1(sptk::ReadStream(&expected, &stream_for_expected));
    const bool result2(sptk::ReadStream(&actual, &stream_for_actual));
    if (result1 != result2) {
      std::cout << "Acutual data length and expected one differ" << std::endl;
      status = -1;
      break;
    }
    if (!result1) {
      break;
    }
    if (tolerance < std::fabs(actual - expected)) {
      std::cout << "[No. " << sample_index << "] is not almost equal"
                << std::endl;
      status = -1;
    }
  }

  return status;
}
