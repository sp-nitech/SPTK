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

#include <algorithm>  // std::transform
#include <cmath>      // std::round, std::sin
#include <fstream>    // std::ifstream
#include <iomanip>    // std::setw
#include <iostream>   // std::cerr, std::cin, std::cout, std::endl, etc.
#include <numeric>    // std::accumulate
#include <sstream>    // std::ostringstream
#include <vector>     // std::vector

#include "GETOPT/ya_getopt.h"
#include "SPTK/analysis/pitch_extraction.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum OutputFormats {
  kBinarySequence = 0,
  kPositionInSeconds,
  kPositionInSamples,
  kSine,
  kCosine,
  kSawtooth,
  kNumOutputFormats
};

const double kDefaultSamplingRate(16.0);
const double kDefaultLowerF0(60.0);
const double kDefaultUpperF0(240.0);
const double kDefaultVoicingThreshold(0.9);
const OutputFormats kDefaultOutputFormat(kBinarySequence);
const double kDefaultUnvoicedValue(0.0);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " pitch_mark - pitch mark extraction" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       pitch_mark [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -s s  : sampling rate [kHz]           (double)[" << std::setw(5) << std::right << kDefaultSamplingRate     << "][  6.0 <  s <= 98.0  ]" << std::endl;  // NOLINT
  *stream << "       -L L  : minimum fundamental frequency (double)[" << std::setw(5) << std::right << kDefaultLowerF0          << "][ 10.0 <  L <  H     ]" << std::endl;  // NOLINT
  *stream << "               to search for [Hz]" << std::endl;
  *stream << "       -H H  : maximum fundamental frequency (double)[" << std::setw(5) << std::right << kDefaultUpperF0          << "][    L <  H <  500*s ]" << std::endl;  // NOLINT
  *stream << "               to search for [Hz]" << std::endl;
  *stream << "       -t t  : voicing threshold             (double)[" << std::setw(5) << std::right << kDefaultVoicingThreshold << "][ -0.5 <= t <= 1.6   ]" << std::endl;  // NOLINT
  *stream << "       -o o  : output format                 (   int)[" << std::setw(5) << std::right << kDefaultOutputFormat     << "][    0 <= o <= 5     ]" << std::endl;  // NOLINT
  *stream << "                 0 (binary sequence)" << std::endl;
  *stream << "                 1 (position in seconds)" << std::endl;
  *stream << "                 2 (position in samples)" << std::endl;
  *stream << "                 3 (sine waveform)" << std::endl;
  *stream << "                 4 (cosine waveform)" << std::endl;
  *stream << "                 5 (sawtooth waveform)" << std::endl;
  *stream << "       -u u  : value on unvoiced region      (double)[" << std::setw(5) << std::right << kDefaultUnvoicedValue    << "][      <= u <=       ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       waveform                              (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       pitch mark                            (double)" << std::endl;  // NOLINT
  *stream << "  notice:" << std::endl;
  *stream << "       if t is raised, the number of pitch marks increase" << std::endl;  // NOLINT
  *stream << "       the value of t should be in the recommended range but values outside the range can be given" << std::endl;  // NOLINT
  *stream << "       if o = 0, value 1 or -1 indicating pitch mark is outputted considering polarity" << std::endl;  // NOLINT
  *stream << "       -u option is valid only o >= 3" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a pitch_mark [ @e option ] [ @e infile ]
 *
 * - @b -s @e double
 *   - sampling rate [kHz] @f$(6 < F_s \le 98)@f$
 * - @b -L @e double
 *   - minimum F0 to search for [Hz] @f$(10 < F_l < F_h)@f$
 * - @b -H @e double
 *   - maximum F0 to search for [Hz] @f$(F_l < F_h < 500F_s)@f$
 * - @b -t @e double
 *   - voicing threshold @f$(-0.5 \le T \le 1.6)@f$
 * - @b -o @e int
 *   - output format
 *     @arg @c 0 binary sequence
 *     @arg @c 1 position in seconds
 *     @arg @c 2 position in samples
 *     @arg @c 3 sine waveform
 *     @arg @c 4 cosine waveform
 *     @arg @c 5 sawtooth waveform
 * - @b -u @e double
 *   - value on unvoiced region
 * - @b infile @e str
 *   - double-type waveform
 * - @b stdout
 *   - double-type pitch mark
 *
 * The below is a simple example to extract pitch marks from @c data.d
 *
 * @code{.sh}
 *   pitch_mark -s 16 -L 80 -H 200 -o 0 < data.d > data.gci
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 *
 * @sa sptk::PitchExtraction
 */
int main(int argc, char* argv[]) {
  double sampling_rate(kDefaultSamplingRate);
  double lower_f0(kDefaultLowerF0);
  double upper_f0(kDefaultUpperF0);
  double voicing_threshold(kDefaultVoicingThreshold);
  OutputFormats output_format(kDefaultOutputFormat);
  double unvoiced_value(kDefaultUnvoicedValue);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "s:L:H:t:o:u:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 's': {
        const double min(6.0);
        const double max(98.0);
        if (!sptk::ConvertStringToDouble(optarg, &sampling_rate) ||
            sampling_rate <= min || max < sampling_rate) {
          std::ostringstream error_message;
          error_message << "The argument for the -s option must be a number "
                        << "in the interval (" << min << ", " << max << "]";
          sptk::PrintErrorMessage("pitch_mark", error_message);
          return 1;
        }
        break;
      }
      case 'L': {
        if (!sptk::ConvertStringToDouble(optarg, &lower_f0) ||
            lower_f0 <= 10.0) {
          std::ostringstream error_message;
          error_message << "The argument for the -L option must be a number "
                        << "greater than 10";
          sptk::PrintErrorMessage("pitch_mark", error_message);
          return 1;
        }
        break;
      }
      case 'H': {
        if (!sptk::ConvertStringToDouble(optarg, &upper_f0) ||
            upper_f0 <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -H option must be a positive number";
          sptk::PrintErrorMessage("pitch_mark", error_message);
          return 1;
        }
        break;
      }
      case 't': {
        if (!sptk::ConvertStringToDouble(optarg, &voicing_threshold)) {
          std::ostringstream error_message;
          error_message << "The argument for the -t option must be numeric";
          sptk::PrintErrorMessage("pitch_mark", error_message);
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
          sptk::PrintErrorMessage("pitch_mark", error_message);
          return 1;
        }
        output_format = static_cast<OutputFormats>(tmp);
        break;
      }
      case 'u': {
        if (!sptk::ConvertStringToDouble(optarg, &unvoiced_value)) {
          std::ostringstream error_message;
          error_message << "The argument for the -u option must be a number";
          sptk::PrintErrorMessage("pitch_mark", error_message);
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

  const double sampling_rate_in_hz(1000.0 * sampling_rate);
  if (0.5 * sampling_rate_in_hz <= upper_f0) {
    std::ostringstream error_message;
    error_message
        << "Upper fundamental frequency must be less than Nyquist frequency";
    sptk::PrintErrorMessage("pitch_mark", error_message);
    return 1;
  }

  if (upper_f0 <= lower_f0) {
    std::ostringstream error_message;
    error_message << "Lower fundamental frequency must be less than upper one";
    sptk::PrintErrorMessage("pitch_mark", error_message);
    return 1;
  }

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("pitch_mark", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  if (!sptk::SetBinaryMode()) {
    std::ostringstream error_message;
    error_message << "Cannot set translation mode";
    sptk::PrintErrorMessage("pitch_mark", error_message);
    return 1;
  }

  std::ifstream ifs;
  if (NULL != input_file) {
    ifs.open(input_file, std::ios::in | std::ios::binary);
    if (ifs.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << input_file;
      sptk::PrintErrorMessage("pitch_mark", error_message);
      return 1;
    }
  }
  std::istream& input_stream(ifs.is_open() ? ifs : std::cin);

  sptk::PitchExtraction pitch_extraction(
      1, sampling_rate_in_hz, lower_f0, upper_f0, voicing_threshold,
      sptk::PitchExtraction::Algorithms::kReaper);
  if (!pitch_extraction.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize PitchExtraction";
    sptk::PrintErrorMessage("pitch_mark", error_message);
    return 1;
  }

  std::vector<double> waveform;
  {
    double tmp;
    while (sptk::ReadStream(&tmp, &input_stream)) {
      waveform.push_back(tmp);
    }
  }
  if (waveform.empty()) return 0;

  const bool waveform_output(kBinarySequence != output_format &&
                             kPositionInSeconds != output_format &&
                             kPositionInSamples != output_format);
  std::vector<double> f0;
  std::vector<double> pitch_mark;
  sptk::PitchExtractionInterface::Polarity polarity;
  if (!pitch_extraction.Run(waveform, waveform_output ? &f0 : NULL, &pitch_mark,
                            &polarity)) {
    std::ostringstream error_message;
    error_message << "Failed to extract pitch mark";
    sptk::PrintErrorMessage("pitch_mark", error_message);
    return 1;
  }

  if (kPositionInSeconds != output_format) {
    std::transform(
        pitch_mark.begin(), pitch_mark.end(), pitch_mark.begin(),
        [sampling_rate_in_hz](double x) { return x * sampling_rate_in_hz; });
  }

  if (sptk::PitchExtractionInterface::Polarity::kUnknown == polarity) {
    std::ostringstream error_message;
    error_message << "Failed to detect polarity";
    sptk::PrintErrorMessage("pitch_mark", error_message);
    return 1;
  }
  const double binary_polarity(
      sptk::PitchExtractionInterface::Polarity::kPositive == polarity ? 1.0
                                                                      : -1.0);
  const int waveform_length(static_cast<int>(waveform.size()));
  const int num_pitch_marks(static_cast<int>(pitch_mark.size()));

  switch (output_format) {
    case kBinarySequence: {
      int next_pitch_mark(pitch_mark.empty()
                              ? -1
                              : static_cast<int>(std::round(pitch_mark[0])));
      for (int i(0), j(1); i < waveform_length; ++i) {
        if (i == next_pitch_mark) {
          if (!sptk::WriteStream(binary_polarity, &std::cout)) {
            std::ostringstream error_message;
            error_message << "Failed to write pitch mark";
            sptk::PrintErrorMessage("pitch_mark", error_message);
            return 1;
          }
          if (j < num_pitch_marks) {
            next_pitch_mark = static_cast<int>(std::round(pitch_mark[j++]));
          }
        } else {
          if (!sptk::WriteStream(0.0, &std::cout)) {
            std::ostringstream error_message;
            error_message << "Failed to write pitch mark";
            sptk::PrintErrorMessage("pitch_mark", error_message);
            return 1;
          }
        }
      }
      break;
    }
    case kPositionInSeconds:
    case kPositionInSamples: {
      if (0 < num_pitch_marks &&
          !sptk::WriteStream(0, num_pitch_marks, pitch_mark, &std::cout,
                             NULL)) {
        std::ostringstream error_message;
        error_message << "Failed to write pitch mark";
        sptk::PrintErrorMessage("pitch_mark", error_message);
        return 1;
      }
      break;
    }
    case kSine:
    case kCosine:
    case kSawtooth: {
      for (int n(0), i(0); n <= num_pitch_marks; ++n) {
        const int next_pitch_mark(
            n < num_pitch_marks ? static_cast<int>(std::round(pitch_mark[n]))
                                : waveform_length);
        // Find the point across voiced region to unvoiced one.
        int j(i);
        for (; j < next_pitch_mark; ++j) {
          if (0.0 == f0[j]) {
            break;
          }
        }

        // Output periodic sequence.
        if (i < j) {
          const double sum_f0(
              std::accumulate(f0.begin() + i, f0.begin() + j, 0.0));
          const double multiplier(sptk::kTwoPi / sum_f0);

          double phase(0.0);
          for (int k(i); k < j; ++k) {
            double value;
            switch (output_format) {
              case kSine: {
                value = std::sin(phase);
                break;
              }
              case kCosine: {
                value = std::cos(phase);
                break;
              }
              case kSawtooth: {
                value = std::fmod(phase, sptk::kTwoPi) / sptk::kPi - 1.0;
                break;
              }
              default: {
                return 1;
              }
            }
            if (!sptk::WriteStream(binary_polarity * value, &std::cout)) {
              std::ostringstream error_message;
              error_message << "Failed to write periodic sequence";
              sptk::PrintErrorMessage("pitch_mark", error_message);
              return 1;
            }
            phase += multiplier * f0[k];
          }
        }

        // Output unvoiced sequence.
        for (int k(j); k < next_pitch_mark; ++k) {
          if (!sptk::WriteStream(unvoiced_value, &std::cout)) {
            std::ostringstream error_message;
            error_message << "Failed to write periodic sequence";
            sptk::PrintErrorMessage("pitch_mark", error_message);
            return 1;
          }
        }

        i = next_pitch_mark;
      }
      break;
    }
    default: {
      return 1;
    }
  }

  return 0;
}
