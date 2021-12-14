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
#include "SPTK/analysis/zero_crossing_analysis.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum OutputFormats {
  kNumZeroCrossing = 0,
  kZeroCrossingRate,
  kNumOutputFormats
};

const int kDefaultFrameLength(256);
const OutputFormats kDefaultOutputFormat(kNumZeroCrossing);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " zcross - zero-crossing detection" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       zcross [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : frame length       (   int)[" << std::setw(5) << std::right << kDefaultFrameLength  << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -o o  : output format      (   int)[" << std::setw(5) << std::right << kDefaultOutputFormat << "][ 0 <= o <= 1 ]" << std::endl;  // NOLINT
  *stream << "                 0 (number of zero-crossings)" << std::endl;
  *stream << "                 1 (zero-crossing rate)" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence              (double)[stdin]" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       zero-crossing              (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a zcross [ @e option ] [ @e infile ]
 *
 * - @b -l @e int
 *   - frame length @f$(1 \le L)@f$
 * - @b -o @e int
 *   - output format
 *     \arg @c 0 number of zero-crossings
 *     \arg @c 1 zero-crossing rate
 * - @b infile @e str
 *   - double-type data sequence
 * - @b stdout
 *   - double-type zero-crossing
 *
 * @code{.sh}
 *   nrand -s 0 -l 30 | zcross -l 10 | x2x +da
 *   # 3, 2, 5
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int frame_length(kDefaultFrameLength);
  OutputFormats output_format(kDefaultOutputFormat);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:o:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &frame_length) ||
            frame_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("zcross", error_message);
          return 1;
        }
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
          sptk::PrintErrorMessage("zcross", error_message);
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

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("zcross", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("zcross", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::ZeroCrossingAnalysis zero_crossing_analysis(frame_length);
  sptk::ZeroCrossingAnalysis::Buffer buffer;
  if (!zero_crossing_analysis.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize ZeroCrossingAnalysis";
    sptk::PrintErrorMessage("zcross", error_message);
    return 1;
  }

  std::vector<double> input_data(frame_length);

  while (sptk::ReadStream(false, 0, 0, frame_length, &input_data, &input_stream,
                          NULL)) {
    int num_zero_crossing;
    if (!zero_crossing_analysis.Run(input_data, &num_zero_crossing, &buffer)) {
      std::ostringstream error_message;
      error_message << "Failed to detect zero-crossing";
      sptk::PrintErrorMessage("zcross", error_message);
      return 1;
    }

    double output(static_cast<double>(num_zero_crossing));
    switch (output_format) {
      case kNumZeroCrossing: {
        // nothing to do
        break;
      }
      case kZeroCrossingRate: {
        output /= frame_length;
        break;
      }
      default: {
        break;
      }
    }

    if (!sptk::WriteStream(output, &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write zero-crossing";
      sptk::PrintErrorMessage("zcross", error_message);
      return 1;
    }
  }

  return 0;
}
