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
//                1996-2018  Nagoya Institute of Technology          //
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

#include "SPTK/utils/linear_predictive_coefficients_stability_check.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum WarningType { kIgnore = 0, kWarn, kExit, kNumWarningTypes };

const int kDefaultNumOrder(25);
const WarningType kDefaultWarningType(kWarn);
const double kDefaultMargin(1e-16);
const bool kDefaultModificationFlag(false);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " lpccheck - check stability of linear predictive coefficients" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       lpccheck [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -m m  : order of coefficients           (   int)[" << std::setw(5) << std::right << kDefaultNumOrder    << "][   0 <= m <=    ]" << std::endl;  // NOLINT
  *stream << "       -e e  : warning type of unstable index  (   int)[" << std::setw(5) << std::right << kDefaultWarningType << "][   0 <= e <= 2  ]" << std::endl;  // NOLINT
  *stream << "                 0 (no warning)" << std::endl;
  *stream << "                 1 (output the index to stderr)" << std::endl;
  *stream << "                 2 (output the index to stderr" << std::endl;
  *stream << "                    and exit immediately)" << std::endl;
  *stream << "       -r r  : margin                          (double)[" << std::setw(5) << std::right << kDefaultMargin      << "][ 0.0 <  r < 1.0 ]" << std::endl;  // NOLINT
  *stream << "       -x    : perform modification            (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultModificationFlag) << "]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       linear predictive coefficients          (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       modified linear predictive coefficients (double)" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);
  WarningType warning_type(kDefaultWarningType);
  double margin(kDefaultMargin);
  bool modification_flag(kDefaultModificationFlag);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "m:e:r:xh", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("lpccheck", error_message);
          return 1;
        }
        break;
      }
      case 'e': {
        const int min(0);
        const int max(static_cast<int>(kNumWarningTypes) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -e option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("lpccheck", error_message);
          return 1;
        }
        warning_type = static_cast<WarningType>(tmp);
        break;
      }
      case 'r': {
        if (!sptk::ConvertStringToDouble(optarg, &margin) || margin <= 0.0 ||
            1.0 <= margin) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -r option must be in [0.0, 1.0]";
          sptk::PrintErrorMessage("lpccheck", error_message);
          return 1;
        }
        break;
      }
      case 'x': {
        modification_flag = true;
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
    sptk::PrintErrorMessage("lpccheck", error_message);
    return 1;
  }
  const char* input_file(0 == num_rest_args ? NULL : argv[optind]);

  // open stream
  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("lpccheck", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  // prepare for stability check
  sptk::LinearPredictiveCoefficientsStabilityCheck
      linear_predictive_coefficients_stability_check(num_order, margin);
  sptk::LinearPredictiveCoefficientsStabilityCheck::Buffer buffer;
  if (!linear_predictive_coefficients_stability_check.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to set condition for stability check";
    sptk::PrintErrorMessage("lpccheck", error_message);
    return 1;
  }

  const int length(num_order + 1);
  std::vector<double> input_linear_predictive_coefficients(length);
  std::vector<double> output_linear_predictive_coefficients(length);
  std::vector<double>* output_ptr(
      modification_flag ? &output_linear_predictive_coefficients : NULL);
  std::vector<double>* write_ptr(modification_flag
                                     ? &output_linear_predictive_coefficients
                                     : &input_linear_predictive_coefficients);

  for (int frame_index(0); sptk::ReadStream(
           false, 0, 0, length, &input_linear_predictive_coefficients,
           &input_stream, NULL);
       ++frame_index) {
    bool is_stable(false);
    if (!linear_predictive_coefficients_stability_check.Run(
            input_linear_predictive_coefficients, output_ptr, &is_stable,
            &buffer)) {
      std::ostringstream error_message;
      error_message
          << "Failed to check stability of linear predictive coefficients";
      sptk::PrintErrorMessage("lpccheck", error_message);
      return 1;
    }

    if (!is_stable && kIgnore != warning_type) {
      std::ostringstream error_message;
      error_message << frame_index << "th frame is unstable";
      sptk::PrintErrorMessage("lpccheck", error_message);
      if (kExit == warning_type) return 1;
    }

    if (!sptk::WriteStream(0, length, *write_ptr, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write linear predictive coefficients";
      sptk::PrintErrorMessage("lpccheck", error_message);
      return 1;
    }
  }

  return 0;
}
