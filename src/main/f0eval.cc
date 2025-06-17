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

#include <cmath>     // std::log2, std::sqrt
#include <fstream>   // std::ifstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream

#include "GETOPT/ya_getopt.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum InputFormats { kPitch = 0, kF0, kLogF0, kNumInputFormats };

enum OutputFormats {
  kF0ErrorAndVuvError = 0,
  kF0Error,
  kVuvError,
  kNumOutputFormats
};

const InputFormats kDefaultInputFormat(kPitch);
const OutputFormats kDefaultOutputFormat(kF0ErrorAndVuvError);
const double kDefaultSamplingRate(16.0);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " f0eval - calculation of F0 metrics" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       f0eval [ options ] file1 [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -q q  : input format        (   int)[" << std::setw(5) << std::right << kDefaultInputFormat  << "][ 0 <= q <= 2 ]" << std::endl;  // NOLINT
  *stream << "                 0 (Fs/F0)" << std::endl;
  *stream << "                 1 (F0)" << std::endl;
  *stream << "                 2 (log F0)" << std::endl;
  *stream << "       -o o  : output format       (   int)[" << std::setw(5) << std::right << kDefaultOutputFormat << "][ 0 <= o <= 2 ]" << std::endl;  // NOLINT
  *stream << "                 0 (F0 RMSE and V/UV error)" << std::endl;
  *stream << "                 1 (F0 RMSE)" << std::endl;
  *stream << "                 2 (V/UV error)" << std::endl;
  *stream << "       -s s  : sampling rate [kHz] (double)[" << std::setw(5) << std::right << kDefaultSamplingRate << "][ 0 <  s <=   ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  file1:" << std::endl;
  *stream << "       pitch                       (double)" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       pitch                       (double)[stdin]" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       metrics                     (double)" << std::endl;
  *stream << "  notice:" << std::endl;
  *stream << "       F0 RMSE is measured in cents" << std::endl;
  *stream << "       V/UV error is measured in percent" << std::endl;
  *stream << "       -q 0 and -q 1 options treat unvoiced frames as 0.0" << std::endl;  // NOLINT
  *stream << "       -q 2 option treats unvoiced frames as -1e+10" << std::endl;  // NOLINT
  *stream << "       -s option is used only when -q 0 option is specified" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a f0eval [ @e option ] file1 [ @e infile ]
 *
 * - @b -q @e int
 *   - input format
 *     @arg @c 0 pitch @f$(F_s / F_0)@f$
 *     @arg @c 1 F0
 *     @arg @c 2 log F0
 * - @b -o @e int
 *   - output format
 *     @arg @c 0 F0 RMSE and V/UV error
 *     @arg @c 1 F0 RMSE
 *     @arg @c 2 V/UV error
 * - @b -s @e double
 *   - sampling rate [kHz] @f$(0 < F_s)@f$
 * - @b file1 @e str
 *   - double-type pitch
 * - @b infile @e str
 *   - double-type pitch
 * - @b stdout
 *   - double-type F0 RMSE [cent] and/or voiced/unvoiced error [%]
 *
 * In the example below, the voiced/unvoiced error [%] between two f0 files
 * @c data1.f0 and @c data2.f0 is evaluated and displayed:
 *
 * @code{.sh}
 *   f0eval -q 1 -o 2 data1.f0 data2.f0 | dmp +d
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  InputFormats input_format(kDefaultInputFormat);
  OutputFormats output_format(kDefaultOutputFormat);
  double sampling_rate(kDefaultSamplingRate);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "q:o:s:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'q': {
        const int min(0);
        const int max(static_cast<int>(kNumInputFormats) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -o option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("f0eval", error_message);
          return 1;
        }
        input_format = static_cast<InputFormats>(tmp);
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
          sptk::PrintErrorMessage("f0eval", error_message);
          return 1;
        }
        output_format = static_cast<OutputFormats>(tmp);
        break;
      }
      case 's': {
        if (!sptk::ConvertStringToDouble(optarg, &sampling_rate) ||
            sampling_rate <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -s option must be a positive number";
          sptk::PrintErrorMessage("f0eval", error_message);
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

  const char* input_file1(NULL);
  const char* input_file2(NULL);
  const int num_input_files(argc - optind);
  if (2 == num_input_files) {
    input_file1 = argv[argc - 2];
    input_file2 = argv[argc - 1];
  } else if (1 == num_input_files) {
    input_file1 = argv[argc - 1];
    input_file2 = NULL;
  } else {
    std::ostringstream error_message;
    error_message << "Just two input files, file1, and infile, are required";
    sptk::PrintErrorMessage("f0eval", error_message);
    return 1;
  }

  if (!sptk::SetBinaryMode()) {
    std::ostringstream error_message;
    error_message << "Cannot set translation mode";
    sptk::PrintErrorMessage("f0eval", error_message);
    return 1;
  }

  std::ifstream ifs1;
  ifs1.open(input_file1, std::ios::in | std::ios::binary);
  if (ifs1.fail()) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file1;
    sptk::PrintErrorMessage("f0eval", error_message);
    return 1;
  }
  std::istream& input_stream1(ifs1);

  std::ifstream ifs2;
  if (NULL != input_file2) {
    ifs2.open(input_file2, std::ios::in | std::ios::binary);
    if (ifs2.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << input_file2;
      sptk::PrintErrorMessage("f0eval", error_message);
      return 1;
    }
  }
  std::istream& input_stream2(ifs2.is_open() ? ifs2 : std::cin);

  const double sampling_rate_in_hz(1000.0 * sampling_rate);
  const double unvoiced_value(kLogF0 == input_format ? sptk::kLogZero : 0.0);

  double f1, f2;
  double f0_error_value(0.0);
  int num_voiced_frame(0);
  int vuv_error_count(0);
  int num_frame(0);

  while (sptk::ReadStream(&f1, &input_stream1) &&
         sptk::ReadStream(&f2, &input_stream2)) {
    ++num_frame;
    if (f1 == unvoiced_value && f2 == unvoiced_value) {
      // nothing to do
    } else if (f1 != unvoiced_value && f2 != unvoiced_value) {
      switch (input_format) {
        case kPitch: {
          f1 = std::log2(sampling_rate_in_hz / f1);
          f2 = std::log2(sampling_rate_in_hz / f2);
          break;
        }
        case kF0: {
          f1 = std::log2(f1);
          f2 = std::log2(f2);
          break;
        }
        case kLogF0: {
          f1 *= sptk::kOctave;
          f2 *= sptk::kOctave;
          break;
        }
        default: {
          return 1;
        }
      }
      const double error_in_cent(1200.0 * (f1 - f2));
      f0_error_value += error_in_cent * error_in_cent;
      ++num_voiced_frame;
    } else {
      ++vuv_error_count;
    }
  }

  if (0 < num_frame) {
    if (kF0ErrorAndVuvError == output_format || kF0Error == output_format) {
      if (0 == num_voiced_frame) {
        std::ostringstream error_message;
        error_message << "There are no voiced frames";
        sptk::PrintErrorMessage("f0eval", error_message);
        return 1;
      }
      const double f0_error(std::sqrt(f0_error_value / num_voiced_frame));
      if (!sptk::WriteStream(f0_error, &std::cout)) {
        std::ostringstream error_message;
        error_message << "Failed to write F0 RMSE";
        sptk::PrintErrorMessage("f0eval", error_message);
        return 1;
      }
    }
    if (kF0ErrorAndVuvError == output_format || kVuvError == output_format) {
      const double vuv_error(100.0 * vuv_error_count / num_frame);
      if (!sptk::WriteStream(vuv_error, &std::cout)) {
        std::ostringstream error_message;
        error_message << "Failed to write V/UV error";
        sptk::PrintErrorMessage("f0eval", error_message);
        return 1;
      }
    }
  }

  return 0;
}
