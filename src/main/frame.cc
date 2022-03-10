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

#include <algorithm>  // std::copy, std::fill, std::transform
#include <fstream>    // std::ifstream
#include <iomanip>    // std::setw
#include <iostream>   // std::cerr, std::cin, std::cout, std::endl, etc.
#include <numeric>    // std::accumulate
#include <sstream>    // std::ostringstream
#include <vector>     // std::vector

#include "Getopt/getoptwin.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum FramingTypes {
  kBegginingOfDataIsCenterOfFirstFrame,
  kBegginingOfDataIsStartOfFirstFrame,
  kNumFramingTypes,
};

const int kDefaultFrameLength(256);
const int kDefaultFramePeriod(100);
const FramingTypes kDefaultFramingType(kBegginingOfDataIsCenterOfFirstFrame);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " frame - extract frame" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       frame [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : frame length       (   int)[" << std::setw(5) << std::right << kDefaultFrameLength << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -p p  : frame period       (   int)[" << std::setw(5) << std::right << kDefaultFramePeriod << "][ 1 <= p <=   ]" << std::endl;  // NOLINT
  *stream << "       -n n  : framing type       (   int)[" << std::setw(5) << std::right << kDefaultFramingType << "][ 0 <= t <= 1 ]" << std::endl;  // NOLINT
  *stream << "                0 (the beginning of data is the center of the first frame)" << std::endl;  // NOLINT
  *stream << "                1 (the beginning of data is the start of the first frame)" << std::endl;  // NOLINT
  *stream << "       -z    : mean subtraction   (  bool)[" << std::setw(5) << std::right << "FALSE"             << "]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence              (double)[stdin]" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       extracted data sequence    (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

bool WriteData(const std::vector<double>& data, bool zero_mean) {
  const int length(static_cast<int>(data.size()));
  if (zero_mean) {
    std::vector<double> processed_data(length);
    const double mean(std::accumulate(data.begin(), data.end(), 0.0) / length);
    std::transform(data.begin(), data.end(), processed_data.begin(),
                   [mean](double x) { return x - mean; });
    if (!sptk::WriteStream(0, length, processed_data, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write data";
      sptk::PrintErrorMessage("frame", error_message);
      return false;
    }
  } else {
    if (!sptk::WriteStream(0, length, data, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write data";
      sptk::PrintErrorMessage("frame", error_message);
      return false;
    }
  }
  return true;
}

}  // namespace

/**
 * @a frame [ @e option ] [ @e infile ]
 *
 * - @b -l @e int
 *   - frame length @f$(1 \le L)@f$
 * - @b -p @e double
 *   - frame period @f$(1 \le P)@f$
 * - @b -n @e int
 *   - framing type
 *     \arg @c 0 the beginning of data is the center of the first frame
 *     \arg @c 1 the beginning of data is the start of the first frame
 * - @b -z @e bool
 *   - perform mean subtraction in a frame
 * - @b infile @e str
 *   - double-type data sequence
 * - @b stdout
 *   - double-type framed data sequence
 *
 * If the input is
 * @f[
 *   \begin{array}{c}
 *     \{x(t)\}_{t=0}^{T-1}
 *   \end{array}
 * @f]
 * and the frame length @f$L@f$ is even, the output is
 * @f[
 *   \begin{array}{cccc}
 *     \{x(t)\}_{t=-L/2}^{t=L/2-1}, &
 *     \{x(t)\}_{t=P-L/2}^{t=P+L/2-1}, &
 *     \{x(t)\}_{t=2P-L/2}^{t=2P+L/2-1}, &
 *     \ldots
 *   \end{array}
 * @f]
 * where @f$P@f$ is frame period and @f$\forall t < 0, t \ge T \; x(t)=0@f$.
 * If <tt>-n 1</tt> is specified, the output is
 * @f[
 *   \begin{array}{cccc}
 *     \{x(t)\}_{t=0}^{t=L-1}, &
 *     \{x(t)\}_{t=P}^{t=P+L-1}, &
 *     \{x(t)\}_{t=2P}^{t=2P+L-1}, &
 *     \ldots
 *   \end{array}
 * @f]
 *
 * The below example extracts LPC coefficients with 25-ms length window and
 * 5-ms shift.
 *
 * @code{.sh}
 *   frame -l 400 -p 80 < data.d | window -l 400 | lpc -l 400 > data.lpc
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int frame_length(kDefaultFrameLength);
  int frame_period(kDefaultFramePeriod);
  FramingTypes framing_type(kDefaultFramingType);
  bool zero_mean(false);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:p:n:zh", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &frame_length) ||
            frame_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("frame", error_message);
          return 1;
        }
        break;
      }
      case 'p': {
        if (!sptk::ConvertStringToInteger(optarg, &frame_period) ||
            frame_period <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -p option must be a positive integer";
          sptk::PrintErrorMessage("frame", error_message);
          return 1;
        }
        break;
      }
      case 'n': {
        const int min(0);
        const int max(static_cast<int>(kNumFramingTypes) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -n option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("frame", error_message);
          return 1;
        }
        framing_type = static_cast<FramingTypes>(tmp);
        break;
      }
      case 'z': {
        zero_mean = true;
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

  const int num_rest_args(argc - optind);
  if (1 < num_rest_args) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("frame", error_message);
    return 1;
  }
  const char* input_file(0 == num_rest_args ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("frame", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  std::vector<double> data(frame_length);
  int actual_read_size;

  // Extract the first frame.
  {
    int read_point;
    int read_size;
    if (kBegginingOfDataIsCenterOfFirstFrame == framing_type) {
      if (0 == frame_length % 2) {
        read_point = frame_length / 2;
        read_size = frame_length / 2;
      } else {
        read_point = (frame_length - 1) / 2;
        read_size = (frame_length + 1) / 2;
      }
    } else if (kBegginingOfDataIsStartOfFirstFrame == framing_type) {
      read_point = 0;
      read_size = frame_length;
    } else {
      return 0;
    }

    if (!sptk::ReadStream(true, 0, read_point, read_size, &data, &input_stream,
                          &actual_read_size)) {
      return 0;
    }
  }

  int center;
  if (kBegginingOfDataIsCenterOfFirstFrame == framing_type) {
    center = frame_length / 2;
  } else if (kBegginingOfDataIsStartOfFirstFrame == framing_type) {
    center = 0;
  } else {
    return 0;
  }

  // Extract the remaining frames.
  const int overlap(frame_length - frame_period);
  if (0 < overlap) {
    bool is_eof(input_stream.peek() == std::ios::traits_type::eof());
    int last_data_position_in_frame(center + actual_read_size - 1);
    while (center <= last_data_position_in_frame) {
      if (is_eof) {
        std::fill(data.begin() + last_data_position_in_frame + 1, data.end(),
                  0.0);
      }

      // Write framed data.
      if (!WriteData(data, zero_mean)) {
        return 1;
      }

      // Move overlapped data.
      std::copy(data.begin() + frame_period, data.end(), data.begin());

      // Read next data.
      if (is_eof) {
        last_data_position_in_frame -= frame_period;
      } else {
        if (!sptk::ReadStream(true, 0, overlap, frame_period, &data,
                              &input_stream, &actual_read_size)) {
          std::ostringstream error_message;
          error_message << "Failed to read data";
          sptk::PrintErrorMessage("frame", error_message);
          return 1;
        }

        if (input_stream.peek() == std::iostream::traits_type::eof()) {
          last_data_position_in_frame = overlap + actual_read_size - 1;
          is_eof = true;
        }
      }
    }
  } else {
    if (!WriteData(data, zero_mean)) {
      return 1;
    }

    while (sptk::ReadStream(true, -overlap, 0, frame_length, &data,
                            &input_stream, &actual_read_size)) {
      if (kBegginingOfDataIsCenterOfFirstFrame == framing_type &&
          actual_read_size <= center) {
        break;
      }
      if (!WriteData(data, zero_mean)) {
        return 1;
      }
    }
  }

  return 0;
}
