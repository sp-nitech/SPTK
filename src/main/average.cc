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
#include "SPTK/math/statistics_accumulation.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kMagicNumberForEndOfFile(-1);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " average - calculation of average" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       average [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : frame length       (   int)[" << std::setw(5) << std::right << "EOF" << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : order of sequence  (   int)[" << std::setw(5) << std::right << "l-1" << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence              (double)[stdin]" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       average                    (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a average [ @e option ] [ @e infile ]
 *
 * - @b -l @e int
 *   - number of items contained in one frame @f$(1 \le L)@f$
 * - @b -m @e int
 *   - order of items contained in one frame @f$(0 \le L - 1)@f$
 * - @b infile @e str
 *   - double-type data sequence
 * - @b stdout
 *   - double-type average
 *
 * The input of this command is
 * @f[
 *   \begin{array}{ccc}
 *     \underbrace{x_0(0), \; \ldots, \; x_0(L-1)}_L, &
 *     \underbrace{x_1(0), \; \ldots, \; x_1(L-1)}_L, &
 *     \ldots,
 *   \end{array}
 * @f]
 * and the output is
 * @f[
 *   \begin{array}{ccc}
 *     a_0, & a_1, & \ldots,
 *   \end{array}
 * @f]
 * where
 * @f[
 *   a_n = \frac{1}{L} \sum_{l=0}^{L-1} x_n(l).
 * @f]
 * If @f$L@f$ is not given, the average of the whole input is computed.
 *
 * @code{.sh}
 *   ramp -l 10 | average | x2x +da
 *   # 4.5
 * @endcode
 *
 * @code{.sh}
 *   ramp -l 10 | average -l 5 | x2x +da
 *   # 2
 *   # 7
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int frame_length(kMagicNumberForEndOfFile);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &frame_length) ||
            frame_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("average", error_message);
          return 1;
        }
        break;
      }
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &frame_length) ||
            frame_length < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("average", error_message);
          return 1;
        }
        ++frame_length;
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
    sptk::PrintErrorMessage("average", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("average", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::StatisticsAccumulation accumulation(0, 1);
  sptk::StatisticsAccumulation::Buffer buffer;
  if (!accumulation.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize StatisticsAccumulation";
    sptk::PrintErrorMessage("average", error_message);
    return 1;
  }

  std::vector<double> data(1);
  for (int data_index(1);
       sptk::ReadStream(false, 0, 0, 1, &data, &input_stream, NULL);
       ++data_index) {
    if (!accumulation.Run(data, &buffer)) {
      std::ostringstream error_message;
      error_message << "Failed to accumulate statistics";
      sptk::PrintErrorMessage("average", error_message);
      return 1;
    }

    if (kMagicNumberForEndOfFile != frame_length &&
        0 == data_index % frame_length) {
      std::vector<double> average(1);
      if (!accumulation.GetMean(buffer, &average)) {
        std::ostringstream error_message;
        error_message << "Failed to compute average";
        sptk::PrintErrorMessage("average", error_message);
        return 1;
      }
      if (!sptk::WriteStream(0, 1, average, &std::cout, NULL)) {
        std::ostringstream error_message;
        error_message << "Failed to write average";
        sptk::PrintErrorMessage("average", error_message);
        return 1;
      }
      accumulation.Clear(&buffer);
    }
  }

  int num_data;
  if (!accumulation.GetNumData(buffer, &num_data)) {
    std::ostringstream error_message;
    error_message << "Failed to accumulate statistics";
    sptk::PrintErrorMessage("average", error_message);
    return 1;
  }

  if (kMagicNumberForEndOfFile == frame_length && 0 < num_data) {
    std::vector<double> average(1);
    if (!accumulation.GetMean(buffer, &average)) {
      std::ostringstream error_message;
      error_message << "Failed to compute average";
      sptk::PrintErrorMessage("average", error_message);
      return 1;
    }
    if (!sptk::WriteStream(0, 1, average, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write average";
      sptk::PrintErrorMessage("average", error_message);
      return 1;
    }
  }

  return 0;
}
