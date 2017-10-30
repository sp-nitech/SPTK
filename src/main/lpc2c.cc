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
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "SPTK/converter/linear_predictive_coefficients_to_cepstrum.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultNumInputOrder(25);
const int kDefaultNumOutputOrder(25);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " lpc2c - transform linear predictive coefficients to cepstrum" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       lpc2c [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -m m  : order of linear predictive (   int)[" << std::setw(5) << std::right << kDefaultNumInputOrder  << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "               coefficients" << std::endl;
  *stream << "       -M M  : order of cepstrum          (   int)[" << std::setw(5) << std::right << kDefaultNumOutputOrder << "][ 0 <= M <=   ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       linear predictive coefficients     (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       cepstrum                           (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

int main(int argc, char* argv[]) {
  int num_input_order(kDefaultNumInputOrder);
  int num_output_order(kDefaultNumOutputOrder);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "m:M:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_input_order) ||
            num_input_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("lpc2c", error_message);
          return 1;
        }
        break;
      }
      case 'M': {
        if (!sptk::ConvertStringToInteger(optarg, &num_output_order) ||
            num_output_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -M option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("lpc2c", error_message);
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

  // get input file
  const int num_rest_args(argc - optind);
  if (1 < num_rest_args) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("lpc2c", error_message);
    return 1;
  }
  const char* input_file(0 == num_rest_args ? NULL : argv[optind]);

  // open stream
  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("lpc2c", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  // prepare for transformation
  sptk::LinearPredictiveCoefficientsToCepstrum
      linear_predictive_coefficients_to_cepstrum(num_input_order,
                                                 num_output_order);
  if (!linear_predictive_coefficients_to_cepstrum.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to set the number of input/output orders";
    sptk::PrintErrorMessage("lpc2c", error_message);
    return 1;
  }

  const int input_length(num_input_order + 1);
  const int output_length(num_output_order + 1);
  std::vector<double> linear_predictive_coefficients(input_length);
  std::vector<double> cepstrum(output_length);

  while (sptk::ReadStream(false, 0, 0, input_length,
                          &linear_predictive_coefficients, &input_stream)) {
    if (!linear_predictive_coefficients_to_cepstrum.Run(
            linear_predictive_coefficients, &cepstrum)) {
      std::ostringstream error_message;
      error_message
          << "Failed to convert linear predictive coefficients to cepstrum";
      sptk::PrintErrorMessage("lpc2c", error_message);
      return 1;
    }

    if (!sptk::WriteStream(0, output_length, cepstrum, &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write cepstrum";
      sptk::PrintErrorMessage("lpc2c", error_message);
      return 1;
    }
  }

  return 0;
}
