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

#include "SPTK/normalizer/generalized_cepstrum_gain_normalization.h"
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
  *stream << "       -m m  : order of generalized cepstrum (   int)[" << std::setw(5) << std::right << kDefaultNumOrder << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -g g  : gamma                         (double)[" << std::setw(5) << std::right << kDefaultGamma    << "][   <= g <=   ]" << std::endl;  // NOLINT
  *stream << "       -c c  : gamma = -1 / c                (   int)[" << std::setw(5) << std::right << "N/A"            << "][ 1 <= c <=   ]" << std::endl;  // NOLINT
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
        if (!sptk::ConvertStringToDouble(optarg, &gamma)) {
          std::ostringstream error_message;
          error_message << "The argument for the -g option must be numeric";
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

  // get input file
  const int num_rest_args(argc - optind);
  if (1 < num_rest_args) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("gnorm", error_message);
    return 1;
  }
  const char* input_file(0 == num_rest_args ? NULL : argv[optind]);

  // open stream
  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("gnorm", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  // prepare for gain normalization
  sptk::GeneralizedCepstrumGainNormalization
      generalized_cepstrum_gain_normalization(num_order, gamma);
  if (!generalized_cepstrum_gain_normalization.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to set condition for normalization";
    sptk::PrintErrorMessage("gnorm", error_message);
    return 1;
  }

  const int length(num_order + 1);
  std::vector<double> generalized_cepstrum(length);
  std::vector<double> normalized_generalized_cepstrum(length);

  while (sptk::ReadStream(false, 0, 0, length, &generalized_cepstrum,
                          &input_stream)) {
    if (!generalized_cepstrum_gain_normalization.Run(
            generalized_cepstrum, &normalized_generalized_cepstrum)) {
      std::ostringstream error_message;
      error_message << "Failed to normalize generalized ceptrum";
      sptk::PrintErrorMessage("gnorm", error_message);
      return 1;
    }

    if (!sptk::WriteStream(length, 0, normalized_generalized_cepstrum,
                           &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to normalized generalized cepstrum";
      sptk::PrintErrorMessage("gnorm", error_message);
      return 1;
    }
  }

  return 0;
}
