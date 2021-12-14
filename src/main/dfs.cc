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

/**
 * @a dfs [ @e option ] [ @e infile ]
 *
 * - @b -a @e int+
 *   - denominator coefficients
 * - @b -b @e int+
 *   - numerator coefficients
 * - @b -p @e str
 *   - file containing denominator coefficients
 * - @b -z @e str
 *   - file containing numerator coefficients
 * - @b infile @e str
 *   - double-type filter input
 * - @b stdout
 *   - double-type filter output
 *
 * The below example applies a pre-emphasis filter to signals in @c data.d
 *
 * @code{.sh}
 *   dfs -a 1 -0.97 < data.d > data.d2
 * @endcode
 *
 * This is equivalent to the following procedure.
 *
 * @code{.sh}
 *   echo 1 -0.97 | x2x +ad > data.p
 *   dfs -p data.p < data.d > data.d2
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
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

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("dfs", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

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

    std::ifstream ifs2;
    ifs2.open(denominator_coefficients_file, std::ios::in | std::ios::binary);
    if (ifs2.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << denominator_coefficients_file;
      sptk::PrintErrorMessage("dfs", error_message);
      return 1;
    }

    double coefficient;
    while (sptk::ReadStream(&coefficient, &ifs2)) {
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

    std::ifstream ifs2;
    ifs2.open(numerator_coefficients_file, std::ios::in | std::ios::binary);
    if (ifs2.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << numerator_coefficients_file;
      sptk::PrintErrorMessage("dfs", error_message);
      return 1;
    }

    double coefficient;
    while (sptk::ReadStream(&coefficient, &ifs2)) {
      numerator_coefficients.push_back(coefficient);
    }
  }
  if (numerator_coefficients.empty()) {
    numerator_coefficients.push_back(1.0);
  }

  sptk::InfiniteImpulseResponseDigitalFilter filter(denominator_coefficients,
                                                    numerator_coefficients);
  sptk::InfiniteImpulseResponseDigitalFilter::Buffer buffer;
  if (!filter.IsValid()) {
    std::ostringstream error_message;
    error_message
        << "Failed to initialize InfiniteImpulseResponseDigitalFilter";
    sptk::PrintErrorMessage("dfs", error_message);
    return 1;
  }

  double signal;

  while (sptk::ReadStream(&signal, &input_stream)) {
    if (!filter.Run(&signal, &buffer)) {
      std::ostringstream error_message;
      error_message << "Failed to apply digital filter";
      sptk::PrintErrorMessage("dfs", error_message);
      return 1;
    }

    if (!sptk::WriteStream(signal, &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write a filter output";
      sptk::PrintErrorMessage("dfs", error_message);
      return 1;
    }
  }

  return 0;
}
