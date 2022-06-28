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
#include "SPTK/input/input_source_delay.h"
#include "SPTK/input/input_source_from_stream.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultVectorLength(1);
const int kDefaultStartIndex(0);
const bool kDefaultKeepSequenceLengthFlag(false);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " delay - delay data sequence" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       delay [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : length of vector     (   int)[" << std::setw(5) << std::right << kDefaultVectorLength << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : order of vector      (   int)[" << std::setw(5) << std::right << "l-1"                << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -s s  : start index          (   int)[" << std::setw(5) << std::right << kDefaultStartIndex   << "][   <= s <=   ]" << std::endl;  // NOLINT
  *stream << "       -k    : keep sequence length (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultKeepSequenceLengthFlag) << "]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence                (double)[stdin]" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       delayed data sequence        (double)" << std::endl;
  *stream << "  notice:" << std::endl;
  *stream << "       if s < 0, advance data sequence" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a delay [ @e option ] [ @e infile ]
 *
 * - @b -l @e int
 *   - length of vector @f$(1 \le L)@f$
 * - @b -m @e int
 *   - order of vector @f$(0 \le M)@f$
 * - @b -s @e int
 *   - start index @f$(S)@f$
 * - @b -k @e double
 *   - keep sequence length
 * - @b infile @e str
 *   - double-type data sequence
 * - @b stdout
 *   - double-type delayed data sequence
 *
 * The input of this command is
 * @f[
 *   \begin{array}{cccc}
 *     x(0), & x(1), & \ldots, & x(T-1)
 *   \end{array}
 * @f]
 * and the output is
 * @f[
 *   \begin{array}{cccc}
 *     \underbrace{0, \; \ldots, \; 0}_S, & x(0), & \ldots, & x(T-1).
 *   \end{array}
 * @f]
 * If @c -k option is specified, the output is
 * @f[
 *   \begin{array}{cccc}
 *     \underbrace{0, \; \ldots, \; 0}_S, & x(0), & \ldots, & x(T-S-1).
 *   \end{array}
 * @f]
 * If @f$S@f$ is negative, the output is
 * @f[
 *   \begin{array}{cccc}
 *     x(-S), & x(-S+1), & \ldots, & x(T-1).
 *   \end{array}
 * @f]
 *
 * See the following examples.
 *
 * @code{.sh}
 *   # data: 1, 2, 3
 *   ramp -s 1 -l 3 | delay -s 2 | x2x +da
 *   # 0, 0, 1, 2, 3
 *   ramp -s 1 -l 3 | delay -s 2 -k | x2x +da
 *   # 0, 0, 1
 *   ramp -s 1 -l 3 | delay -s -2 | x2x +da
 *   # 3
 *   ramp -s 1 -l 3 | delay -s -2 -k | x2x +da
 *   # 3, 0, 0
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int vector_length(kDefaultVectorLength);
  int start_index(kDefaultStartIndex);
  bool keep_sequence_length_flag(kDefaultKeepSequenceLengthFlag);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:s:kh", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &vector_length) ||
            vector_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("delay", error_message);
          return 1;
        }
        break;
      }
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &vector_length) ||
            vector_length < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("delay", error_message);
          return 1;
        }
        ++vector_length;
        break;
      }
      case 's': {
        if (!sptk::ConvertStringToInteger(optarg, &start_index)) {
          std::ostringstream error_message;
          error_message << "The argument for the -s option must be an integer";
          sptk::PrintErrorMessage("delay", error_message);
          return 1;
        }
        break;
      }
      case 'k': {
        keep_sequence_length_flag = true;
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
    sptk::PrintErrorMessage("delay", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("delay", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::InputSourceFromStream input_source(false, vector_length, &input_stream);
  if (!input_source.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize InputSourceFromStream";
    sptk::PrintErrorMessage("delay", error_message);
    return 1;
  }

  sptk::InputSourceDelay input_source_delay(
      start_index, keep_sequence_length_flag, &input_source);
  if (!input_source_delay.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize InputSourceDelay";
    sptk::PrintErrorMessage("delay", error_message);
    return 1;
  }

  std::vector<double> data;
  while (input_source_delay.Get(&data)) {
    if (!sptk::WriteStream(0, vector_length, data, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write delayed data";
      sptk::PrintErrorMessage("delay", error_message);
      return 1;
    }
  }

  return 0;
}
