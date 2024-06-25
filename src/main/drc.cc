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

#include "GETOPT/ya_getopt.h"
#include "SPTK/compression/dynamic_range_compression.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const double kDefaultAbsMaxValue(32768.0);
const double kDefaultSamplingRate(16.0);
const double kDefaultThreshold(-20);
const double kDefaultRatio(2.0);
const double kDefaultKneeWidth(0.0);
const double kDefaultAttackTime(1.0);
const double kDefaultReleaseTime(500.0);
const double kDefaultMakeUpGain(0.0);
const sptk::DynamicRangeCompression::DetectorType kDefaultDetectorType(
    sptk::DynamicRangeCompression::DetectorType::kDecoupled);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " drc - dynamic range compression" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       drc [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -v v  : absolute maximum of input (double)[" << std::setw(5) << std::right << kDefaultAbsMaxValue  << "][ 0.0 <  v <=     ]" << std::endl;  // NOLINT
  *stream << "       -s s  : sampling rate [kHz]       (double)[" << std::setw(5) << std::right << kDefaultSamplingRate << "][ 0.0 <  s <=     ]" << std::endl;  // NOLINT
  *stream << "       -t t  : threshold [dB]            (double)[" << std::setw(5) << std::right << kDefaultThreshold    << "][     <= t <= 0.0 ]" << std::endl;  // NOLINT
  *stream << "       -r r  : input/output ratio        (double)[" << std::setw(5) << std::right << kDefaultRatio        << "][ 1.0 <= r <=     ]" << std::endl;  // NOLINT
  *stream << "       -w w  : knee width [dB]           (double)[" << std::setw(5) << std::right << kDefaultKneeWidth    << "][ 0.0 <= w <=     ]" << std::endl;  // NOLINT
  *stream << "       -A A  : attack time [msec]        (double)[" << std::setw(5) << std::right << kDefaultAttackTime   << "][ 0.0 <  A <=     ]" << std::endl;  // NOLINT
  *stream << "       -R R  : release time [msec]       (double)[" << std::setw(5) << std::right << kDefaultReleaseTime  << "][ 0.0 <  R <=     ]" << std::endl;  // NOLINT
  *stream << "       -m m  : make-up gain              (double)[" << std::setw(5) << std::right << kDefaultMakeUpGain   << "][ 0.0 <= m <=     ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "     (level 2)" << std::endl;
  *stream << "       -d d  : peak detector type        (   int)[" << std::setw(5) << std::right << kDefaultDetectorType << "][   0 <= d <= 3   ]" << std::endl;  // NOLINT
  *stream << "                 0 (branching)" << std::endl;
  *stream << "                 1 (decoupled)" << std::endl;
  *stream << "                 2 (branching, smooth)" << std::endl;
  *stream << "                 3 (decoupled, smooth)" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       input sequence                    (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       compressed sequence               (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a drc [ @e option ] [ @e infile ]
 *
 * - @b -v @e double
 *   - absolute maximum value of input @f$(0 < V)@f$
 * - @b -s @e double
 *   - sampling rate [kHz] @f$(0 < F_s)@f$
 * - @b -t @e double
 *   - threshold [dB] @f$(T \le 0)@f$
 * - @b -r @e double
 *   - ratio @f$(1 \le R)@f$
 * - @b -w @e double
 *   - knee width [dB] @f$(0 \le W)@f$
 * - @b -A @e double
 *   - attack time [msec] @f$(0 < \tau_A)@f$
 * - @b -R @e double
 *   - release time [msec] @f$(0 < \tau_R)@f$
 * - @b -m @e double
 *   - make-up gain @f$(0 \le M)@f$
 * - @b -d @e int
 *   - peak detector type
 *     \arg @c 0 branching
 *     \arg @c 1 decoupled
 *     \arg @c 2 branching (smooth)
 *     \arg @c 3 decoupled (smooth)
 * - @b infile @e str
 *   - double-type input data sequence
 * - @b stdout
 *   - double-type compressed data sequence
 *
 * The below example applies DRC to 16-bit speech waveform.
 *
 * @code{.sh}
 *   x2x +sd data.short | drc -v 32768 | x2x +ds -r | \
 *     sox -c 1 -t s16 -r 16000 - -t wav output.wav
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  double abs_max_value(kDefaultAbsMaxValue);
  double sampling_rate(kDefaultSamplingRate);
  double threshold(kDefaultThreshold);
  double ratio(kDefaultRatio);
  double knee_width(kDefaultKneeWidth);
  double attack_time(kDefaultAttackTime);
  double release_time(kDefaultReleaseTime);
  double makeup_gain(kDefaultMakeUpGain);
  sptk::DynamicRangeCompression::DetectorType detector_type(
      kDefaultDetectorType);

  for (;;) {
    const int option_char(
        getopt_long(argc, argv, "v:s:t:r:w:A:R:m:d:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'v': {
        if (!sptk::ConvertStringToDouble(optarg, &abs_max_value) ||
            abs_max_value <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -v option must be a positive number";
          sptk::PrintErrorMessage("drc", error_message);
          return 1;
        }
        break;
      }
      case 's': {
        if (!sptk::ConvertStringToDouble(optarg, &sampling_rate) ||
            sampling_rate <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -s option must be a positive number";
          sptk::PrintErrorMessage("drc", error_message);
          return 1;
        }
        break;
      }
      case 't': {
        if (!sptk::ConvertStringToDouble(optarg, &threshold)) {
          std::ostringstream error_message;
          error_message << "The argument for the -t option must be a number";
          sptk::PrintErrorMessage("drc", error_message);
          return 1;
        }
        break;
      }
      case 'r': {
        if (!sptk::ConvertStringToDouble(optarg, &ratio) || ratio < 1.0) {
          std::ostringstream error_message;
          error_message << "The argument for the -r option must be a number "
                        << "greater than or equal to one";
          sptk::PrintErrorMessage("drc", error_message);
          return 1;
        }
        break;
      }
      case 'w': {
        if (!sptk::ConvertStringToDouble(optarg, &knee_width) ||
            knee_width < 0.0) {
          std::ostringstream error_message;
          error_message << "The argument for the -w option must be a "
                        << "non-negative number";
          sptk::PrintErrorMessage("drc", error_message);
          return 1;
        }
        break;
      }
      case 'A': {
        if (!sptk::ConvertStringToDouble(optarg, &attack_time) ||
            attack_time <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -A option must be a positive number";
          sptk::PrintErrorMessage("drc", error_message);
          return 1;
        }
        break;
      }
      case 'R': {
        if (!sptk::ConvertStringToDouble(optarg, &release_time) ||
            release_time <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -R option must be a positive number";
          sptk::PrintErrorMessage("drc", error_message);
          return 1;
        }
        break;
      }
      case 'm': {
        if (!sptk::ConvertStringToDouble(optarg, &makeup_gain) ||
            makeup_gain < 0.0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative number";
          sptk::PrintErrorMessage("drc", error_message);
          return 1;
        }
        break;
      }
      case 'd': {
        const int min(0);
        const int max(static_cast<int>(sptk::DynamicRangeCompression::
                                           DetectorType::kNumDetectorTypes) -
                      1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -d option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("drc", error_message);
          return 1;
        }
        detector_type =
            static_cast<sptk::DynamicRangeCompression::DetectorType>(tmp);
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
    sptk::PrintErrorMessage("drc", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  if (!sptk::SetBinaryMode()) {
    std::ostringstream error_message;
    error_message << "Cannot set translation mode";
    sptk::PrintErrorMessage("drc", error_message);
    return 1;
  }

  std::ifstream ifs;
  if (NULL != input_file) {
    ifs.open(input_file, std::ios::in | std::ios::binary);
    if (ifs.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << input_file;
      sptk::PrintErrorMessage("drc", error_message);
      return 1;
    }
  }
  std::istream& input_stream(ifs.is_open() ? ifs : std::cin);

  sptk::DynamicRangeCompression dynamic_range_compression(
      abs_max_value, 1000.0 * sampling_rate, threshold, ratio, knee_width,
      attack_time, release_time, makeup_gain, detector_type);
  sptk::DynamicRangeCompression::Buffer buffer;
  if (!dynamic_range_compression.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize DynamicRangeCompression";
    sptk::PrintErrorMessage("drc", error_message);
    return 1;
  }

  double signal;

  while (sptk::ReadStream(&signal, &input_stream)) {
    if (!dynamic_range_compression.Run(&signal, &buffer)) {
      std::ostringstream error_message;
      error_message << "Failed to compress signal";
      sptk::PrintErrorMessage("drc", error_message);
      return 1;
    }

    if (!sptk::WriteStream(signal, &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write compressed signal";
      sptk::PrintErrorMessage("drc", error_message);
      return 1;
    }
  }

  return 0;
}
