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

#include "SPTK/filter/infinite_impulse_response_digital_filter.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " dfs - infinite impulse response digital filter" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       dfs [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -a K  a1 ... aN : denominator coefficients (double)[" << std::setw(5) << std::right << "1"   << "]" << std::endl;  // NOLINT
  *stream << "       -b b0 b1 ... bM : numerator coefficients   (double)[" << std::setw(5) << std::right << "1"   << "]" << std::endl;  // NOLINT
  *stream << "       -p p            : name of file containing  (string)[" << std::setw(5) << std::right << "N/A" << "]" << std::endl;  // NOLINT
  *stream << "                         denominator coefficients" << std::endl;
  *stream << "       -z z            : name of file containing  (string)[" << std::setw(5) << std::right << "N/A" << "]" << std::endl;  // NOLINT
  *stream << "                         numerator coefficients" << std::endl;
  *stream << "       -h              : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       filter input                               (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       filter output                              (double)" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

int main(int argc, char* argv[]) {
  std::vector<double> denominator_coefficients;
  std::vector<double> numerator_coefficients;
  const char* denominator_coefficients_file(NULL);
  const char* numerator_coefficients_file(NULL);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "a:b:p:z:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'a': {
        denominator_coefficients.clear();
        double coefficient;
        if (!sptk::ConvertStringToDouble(optarg, &coefficient)) {
          std::ostringstream error_message;
          error_message << "The argument for the -a option must be numeric";
          sptk::PrintErrorMessage("dfs", error_message);
          return 1;
        }
        denominator_coefficients.push_back(coefficient);
        while (optind < argc &&
               sptk::ConvertStringToDouble(argv[optind], &coefficient)) {
          denominator_coefficients.push_back(coefficient);
          ++optind;
        }
        break;
      }
      case 'b': {
        numerator_coefficients.clear();
        double coefficient;
        if (!sptk::ConvertStringToDouble(optarg, &coefficient)) {
          std::ostringstream error_message;
          error_message << "The argument for the -b option must be numeric";
          sptk::PrintErrorMessage("dfs", error_message);
          return 1;
        }
        numerator_coefficients.push_back(coefficient);
        while (optind < argc &&
               sptk::ConvertStringToDouble(argv[optind], &coefficient)) {
          numerator_coefficients.push_back(coefficient);
          ++optind;
        }
        break;
      }
      case 'p': {
        denominator_coefficients_file = optarg;
        break;
      }
      case 'z': {
        numerator_coefficients_file = optarg;
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
  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("dfs", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  // open stream
  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("dfs", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  if (NULL != denominator_coefficients_file) {
    if (!denominator_coefficients.empty()) {
      std::ostringstream error_message;
      error_message << "Cannot specify -a and -p options at the same time";
      sptk::PrintErrorMessage("dfs", error_message);
      return 1;
    }

    std::ifstream ifs;
    ifs.open(denominator_coefficients_file, std::ios::in | std::ios::binary);
    if (ifs.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << denominator_coefficients_file;
      sptk::PrintErrorMessage("dfs", error_message);
      return 1;
    }

    double coefficient;
    while (sptk::ReadStream(&coefficient, &ifs)) {
      denominator_coefficients.push_back(coefficient);
    }
  }
  if (denominator_coefficients.empty()) {
    denominator_coefficients.push_back(1.0);
  }

  if (NULL != numerator_coefficients_file) {
    if (!numerator_coefficients.empty()) {
      std::ostringstream error_message;
      error_message << "Cannot specify -b and -z options at the same time";
      sptk::PrintErrorMessage("dfs", error_message);
      return 1;
    }

    std::ifstream ifs;
    ifs.open(numerator_coefficients_file, std::ios::in | std::ios::binary);
    if (ifs.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << numerator_coefficients_file;
      sptk::PrintErrorMessage("dfs", error_message);
      return 1;
    }

    double coefficient;
    while (sptk::ReadStream(&coefficient, &ifs)) {
      numerator_coefficients.push_back(coefficient);
    }
  }
  if (numerator_coefficients.empty()) {
    numerator_coefficients.push_back(1.0);
  }

  double filter_input, filter_output;
  sptk::InfiniteImpulseResponseDigitalFilter filter(denominator_coefficients,
                                                    numerator_coefficients);
  sptk::InfiniteImpulseResponseDigitalFilter::Buffer buffer;
  if (!filter.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to set condition for filtering";
    sptk::PrintErrorMessage("dfs", error_message);
    return 1;
  }

  while (sptk::ReadStream(&filter_input, &input_stream)) {
    if (!filter.Run(filter_input, &filter_output, &buffer)) {
      std::ostringstream error_message;
      error_message << "Failed to apply digital filter";
      sptk::PrintErrorMessage("dfs", error_message);
      return 1;
    }

    if (!sptk::WriteStream(filter_output, &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write a filter output";
      sptk::PrintErrorMessage("dfs", error_message);
      return 1;
    }
  }

  return 0;
}
