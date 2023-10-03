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

#include <cmath>     // std::cos, std::sin
#include <fstream>   // std::ifstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream

#include "GETOPT/ya_getopt.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum OutputFormats { kSine = 0, kCosine, kNumOutputFormats };

const OutputFormats kDefaultOutputFormat(kSine);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " pitch_mark2sin - convert pitch marks to sinusoidal sequence" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       pitch_mark2sin [ options ] vufile [ infile ] > stdout" << std::endl;  // NOLINT
  *stream << "  options:" << std::endl;
  *stream << "       -o o  : output format      (   int)[" << std::setw(5) << std::right << kDefaultOutputFormat << "][ 0 <= o <= 1 ]" << std::endl;  // NOLINT
  *stream << "                 0 (sine)" << std::endl;
  *stream << "                 1 (cosine)" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  vufile:" << std::endl;
  *stream << "       voiced/unvoiced symbol     (double)" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       binary pitch mark          (double)[stdin]" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       sinusoidal sequence        (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a pitch_mark2sin [ @e option ] @e vufile [ @e infile ]
 *
 * - @b -o @e int
 *   - output format
 *     \arg @c 0 sine
 *     \arg @c 1 cosine
 * - @b vufile @e str
 *   - double-type voiced/unvoiced symbol
 * - @b infile @e str
 *   - double-type pitch mark
 * - @b stdout
 *   - double-type sinusoidal sequence
 *
 * The below is a simple example to generate sinusoidal from @c data.d.
 *
 * @code{.sh}
 *   pitch_mark -s 16 -o 0 < data.d | pitch_mark2sin > data.sin
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  OutputFormats output_format(kDefaultOutputFormat);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "o:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'o': {
        const int min(0);
        const int max(static_cast<int>(kNumOutputFormats) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -o option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("pitch_mark2sin", error_message);
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

  const char* vuv_file;
  const char* input_file;
  const int num_input_files(argc - optind);
  if (2 == num_input_files) {
    vuv_file = argv[argc - 2];
    input_file = argv[argc - 1];
  } else if (1 == num_input_files) {
    vuv_file = argv[argc - 1];
    input_file = NULL;
  } else {
    std::ostringstream error_message;
    error_message << "Just two input files, vufile and infile, are required";
    sptk::PrintErrorMessage("pitch_mark2sin", error_message);
    return 1;
  }

  if (!sptk::SetBinaryMode()) {
    std::ostringstream error_message;
    error_message << "Cannot set translation mode";
    sptk::PrintErrorMessage("pitch_mark2sin", error_message);
    return 1;
  }

  std::vector<double> pitch_mark;
  {
    std::ifstream ifs;
    if (NULL != input_file) {
      ifs.open(input_file, std::ios::in | std::ios::binary);
      if (ifs.fail()) {
        std::ostringstream error_message;
        error_message << "Cannot open file " << input_file;
        sptk::PrintErrorMessage("pitch_mark2sin", error_message);
        return 1;
      }
    }
    std::istream& input_stream(ifs.is_open() ? ifs : std::cin);

    double tmp;
    while (sptk::ReadStream(&tmp, &input_stream)) {
      pitch_mark.push_back(tmp);
    }
  }

  std::vector<double> vuv;
  {
    std::ifstream ifs;
    ifs.open(vuv_file, std::ios::in | std::ios::binary);
    if (ifs.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << vuv_file;
      sptk::PrintErrorMessage("pitch_mark2sin", error_message);
      return 1;
    }
    std::istream& input_stream(ifs);

    double tmp;
    while (sptk::ReadStream(&tmp, &input_stream)) {
      vuv.push_back(tmp);
    }
  }

  const int length(static_cast<int>(std::min(pitch_mark.size(), vuv.size())));
  if (0 == length) return 0;

  bool voiced_region(0.0 != pitch_mark[0]);
  int curr(0);
  while (curr < length) {
    int next(curr + 1);
    bool sin_output(false);
    for (; next < length; ++next) {
      if (voiced_region) {
        if (0.0 != pitch_mark[next]) {
          sin_output = true;
          break;
        } else if (vuv[next] < 0.5) {
          sin_output = true;
          voiced_region = false;
          break;
        }
      } else {
        if (0.0 != pitch_mark[next]) {
          voiced_region = true;
          break;
        }
      }
    }

    const int period(next - curr);
    for (int i(0); i < period; ++i) {
      double output(0.0);
      if (sin_output) {
        const double omega(sptk::kTwoPi * i / period);
        switch (output_format) {
          case kSine: {
            output = std::sin(omega);
            break;
          }
          case kCosine: {
            output = std::cos(omega);
            break;
          }
          default: {
            return 1;
          }
        }
        if (pitch_mark[curr] < 0.0) {
          output = -output;
        }
      }
      if (!sptk::WriteStream(output, &std::cout)) {
        std::ostringstream error_message;
        error_message << "Failed to write sinusoidal sequence";
        sptk::PrintErrorMessage("pitch_mark2sin", error_message);
        return 1;
      }
    }

    curr = next;
  }

  return 0;
}
