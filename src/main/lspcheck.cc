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
#include <cmath>      // std::log
#include <fstream>    // std::ifstream
#include <iomanip>    // std::setw
#include <iostream>   // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>    // std::ostringstream
#include <vector>     // std::vector

#include "Getopt/getoptwin.h"
#include "SPTK/check/line_spectral_pairs_stability_check.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum GainType { kLinearGain = 0, kLogGain, kWithoutGain, kNumGainTypes };

enum InputOutputFormats {
  kFrequencyInRadians = 0,
  kFrequencyInCycles,
  kFrequencyInkHz,
  kFrequencyInHz,
  kNumInputOutputFormats
};

enum WarningType { kIgnore = 0, kWarn, kExit, kNumWarningTypes };

const int kDefaultNumOrder(25);
const double kDefaultSamplingFrequency(10.0);
const GainType kDefaultGainType(kLinearGain);
const InputOutputFormats kDefaultInputFormat(kFrequencyInRadians);
const WarningType kDefaultWarningType(kWarn);
const double kDefaultDistanceRate(0.0);
const double kDefaultMinimumGain(1e-10);
const bool kDefaultModificationFlag(false);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " lspcheck - check stability of line spectral pairs" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       lspcheck [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -m m  : order of line spectral pairs          (   int)[" << std::setw(5) << std::right << kDefaultNumOrder          << "][   0 <= m <=     ]" << std::endl;  // NOLINT
  *stream << "       -s s  : sampling frequency                    (double)[" << std::setw(5) << std::right << kDefaultSamplingFrequency << "][ 0.0 <  s <=     ]" << std::endl;  // NOLINT
  *stream << "       -k k  : gain type                             (   int)[" << std::setw(5) << std::right << kDefaultGainType          << "][   0 <= k <= 2   ]" << std::endl;  // NOLINT
  *stream << "                 0 (linear gain)" << std::endl;
  *stream << "                 1 (log gain)" << std::endl;
  *stream << "                 2 (without gain)" << std::endl;
  *stream << "       -q q  : input format                          (   int)[" << std::setw(5) << std::right << kDefaultInputFormat       << "][   0 <= q <= 3   ]" << std::endl;  // NOLINT
  *stream << "                 0 (frequency [rad])" << std::endl;
  *stream << "                 1 (frequency [cyc])" << std::endl;
  *stream << "                 2 (frequency [kHz])" << std::endl;
  *stream << "                 3 (frequency [Hz])" << std::endl;
  *stream << "       -o o  : output format                         (   int)[" << std::setw(5) << std::right << "q"                       << "][   0 <= o <= 3   ]" << std::endl;  // NOLINT
  *stream << "                 0 (frequency [rad])" << std::endl;
  *stream << "                 1 (frequency [cyc])" << std::endl;
  *stream << "                 2 (frequency [kHz])" << std::endl;
  *stream << "                 3 (frequency [Hz])" << std::endl;
  *stream << "       -e e  : warning type of unstable index        (   int)[" << std::setw(5) << std::right << kDefaultWarningType       << "][   0 <= e <= 2   ]" << std::endl;  // NOLINT
  *stream << "                 0 (no warning)" << std::endl;
  *stream << "                 1 (output the index to stderr)" << std::endl;
  *stream << "                 2 (output the index to stderr and" << std::endl;
  *stream << "                    exit immediately)" << std::endl;
  *stream << "       -r r  : rate of distance between two adjacent (double)[" << std::setw(5) << std::right << kDefaultDistanceRate      << "][ 0.0 <= r <= 1.0 ]" << std::endl;  // NOLINT
  *stream << "               line spectral frequencies" << std::endl;
  *stream << "       -g g  : minimum gain on linear scale          (double)[" << std::setw(5) << std::right << kDefaultMinimumGain       << "][ 0.0 <  g <=     ]" << std::endl;  // NOLINT
  *stream << "       -x    : perform modification                  (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultModificationFlag) << "]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       line spectral pairs                           (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       modified line spectral pairs                  (double)" << std::endl;  // NOLINT
  *stream << "  notice:" << std::endl;
  *stream << "       if k is 2, input length in a frame is assumed to be m instead of m+1" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

double GetScale(InputOutputFormats format, double sampling_frequency) {
  switch (format) {
    case kFrequencyInRadians: {
      return 1.0;
    }
    case kFrequencyInCycles: {
      return sptk::kTwoPi;
    }
    case kFrequencyInkHz: {
      return sptk::kTwoPi / sampling_frequency;
    }
    case kFrequencyInHz: {
      return sptk::kTwoPi * 0.001 / sampling_frequency;
    }
    default: {
      return 1.0;
    }
  }
}

}  // namespace

/**
 * @a lspcheck [ @e option ] [ @e infile ]
 *
 * - @b -m @e int
 *   - order of line spectral pairs @f$(0 \le M)@f$
 * - @b -s @e double
 *   - sampling rate @f$(0 < F_s)@f$
 * - @b -k @e int
 *   - input gain type
 *     \arg @c 0 linear gain
 *     \arg @c 1 log gain
 *     \arg @c 2 without gain
 * - @b -q @e int
 *   - input format
 *     \arg @c 0 frequency in rad
 *     \arg @c 1 frequency in cyc
 *     \arg @c 2 frequency in kHz
 *     \arg @c 3 frequency in Hz
 * - @b -o @e int
 *   - output format
 *     \arg @c 0 frequency in rad
 *     \arg @c 1 frequency in cyc
 *     \arg @c 2 frequency in kHz
 *     \arg @c 3 frequency in Hz
 * - @b -e @e int
 *   - warning type
 *     \arg @c 0 no warning
 *     \arg @c 1 output index
 *     \arg @c 2 output index and exit immediately
 * - @b -r @e double
 *   - rate of distance between adjacent LSPs @f$(0 \le R \le 1)@f$
 * - @b -g @e double
 *   - minimum gain on linear scale @f$(0 < G)@f$
 * - @b -x @e bool
 *   - perform modification
 * - @b infile @e str
 *   - double-type LSP coefficients
 * - @b stdout
 *   - double-type modified LSP coefficients
 *
 * In the following example, 9-th order LSP coefficients in @c data.lsp are
 * modified so that the distance between two adjacent LSPs is greater than
 * @f$0.01\pi/(M+1)@f$.
 *
 * @code{.sh}
 *   lspcheck -m 9 -r 0.01 -x < data.lsp > data2.lsp
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);
  double sampling_frequency(kDefaultSamplingFrequency);
  GainType gain_type(kDefaultGainType);
  InputOutputFormats input_format(kDefaultInputFormat);
  InputOutputFormats output_format(kDefaultInputFormat);
  bool is_output_format_specified(false);
  WarningType warning_type(kDefaultWarningType);
  double distance_rate(kDefaultDistanceRate);
  double minimum_gain(kDefaultMinimumGain);
  bool modification_flag(kDefaultModificationFlag);

  for (;;) {
    const int option_char(
        getopt_long(argc, argv, "m:s:k:q:o:e:r:g:xh", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("lspcheck", error_message);
          return 1;
        }
        break;
      }
      case 's': {
        if (!sptk::ConvertStringToDouble(optarg, &sampling_frequency) ||
            sampling_frequency <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -s option must be a positive number";
          sptk::PrintErrorMessage("lspcheck", error_message);
          return 1;
        }
        break;
      }
      case 'k': {
        const int min(0);
        const int max(static_cast<int>(kNumGainTypes) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -k option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("lspcheck", error_message);
          return 1;
        }
        gain_type = static_cast<GainType>(tmp);
        break;
      }
      case 'q': {
        const int min(0);
        const int max(static_cast<int>(kNumInputOutputFormats) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -q option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("lspcheck", error_message);
          return 1;
        }
        input_format = static_cast<InputOutputFormats>(tmp);
        break;
      }
      case 'o': {
        const int min(0);
        const int max(static_cast<int>(kNumInputOutputFormats) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -o option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("lspcheck", error_message);
          return 1;
        }
        output_format = static_cast<InputOutputFormats>(tmp);
        is_output_format_specified = true;
        break;
      }
      case 'e': {
        const int min(0);
        const int max(static_cast<int>(kNumWarningTypes) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -e option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("lspcheck", error_message);
          return 1;
        }
        warning_type = static_cast<WarningType>(tmp);
        break;
      }
      case 'r': {
        const double min(0.0);
        const double max(1.0);
        if (!sptk::ConvertStringToDouble(optarg, &distance_rate) ||
            !sptk::IsInRange(distance_rate, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -r option must be a number "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("lspcheck", error_message);
          return 1;
        }
        break;
      }
      case 'g': {
        if (!sptk::ConvertStringToDouble(optarg, &minimum_gain) ||
            minimum_gain <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -g option must be a positive number";
          sptk::PrintErrorMessage("lspcheck", error_message);
          return 1;
        }
        break;
      }
      case 'x': {
        modification_flag = true;
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

  if (!is_output_format_specified) {
    output_format = input_format;
  }

  if (kLogGain == gain_type) {
    minimum_gain = std::log(minimum_gain);
  }

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("lspcheck", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("lspcheck", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  const double minimum_distance(distance_rate * sptk::kPi / (num_order + 1));
  sptk::LineSpectralPairsStabilityCheck line_spectral_pairs_stability_check(
      num_order, minimum_distance);
  if (!line_spectral_pairs_stability_check.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize LineSpectralPairsStabilityCheck";
    sptk::PrintErrorMessage("lspcheck", error_message);
    return 1;
  }

  const int length(num_order + 1);
  const int read_write_size(kWithoutGain == gain_type ? num_order : length);
  const int read_write_point(kWithoutGain == gain_type ? 1 : 0);
  std::vector<double> line_spectral_pairs(length);
  const double input_scale(GetScale(input_format, sampling_frequency));
  const double output_scale(1.0 / GetScale(output_format, sampling_frequency));

  for (int frame_index(0);
       sptk::ReadStream(false, 0, read_write_point, read_write_size,
                        &line_spectral_pairs, &input_stream, NULL);
       ++frame_index) {
    // Convert unit to radians.
    if (1.0 != input_scale) {
      std::transform(line_spectral_pairs.begin() + 1, line_spectral_pairs.end(),
                     line_spectral_pairs.begin() + 1,
                     [input_scale](double w) { return w * input_scale; });
    }

    // Check lines.
    bool is_stable(false);
    if (modification_flag) {
      if (!line_spectral_pairs_stability_check.Run(&line_spectral_pairs,
                                                   &is_stable)) {
        std::ostringstream error_message;
        error_message << "Failed to check stability of line spectral pairs";
        sptk::PrintErrorMessage("lspcheck", error_message);
        return 1;
      }
    } else {
      if (!line_spectral_pairs_stability_check.Run(line_spectral_pairs, NULL,
                                                   &is_stable)) {
        std::ostringstream error_message;
        error_message << "Failed to check stability of line spectral pairs";
        sptk::PrintErrorMessage("lspcheck", error_message);
        return 1;
      }
    }
    if (!is_stable && kIgnore != warning_type) {
      std::ostringstream error_message;
      error_message << frame_index << "th frame is unstable";
      sptk::PrintErrorMessage("lspcheck", error_message);
      if (kExit == warning_type) return 1;
    }

    // Check gain.
    if (kWithoutGain != gain_type && line_spectral_pairs[0] < minimum_gain) {
      if (modification_flag) {
        line_spectral_pairs[0] = minimum_gain;
      }
      if (kIgnore != warning_type) {
        std::ostringstream error_message;
        error_message << frame_index << "th frame's gain is too small";
        sptk::PrintErrorMessage("lspcheck", error_message);
        if (kExit == warning_type) return 1;
      }
    }

    // Convert unit from radians.
    if (1.0 != output_scale) {
      std::transform(line_spectral_pairs.begin() + 1, line_spectral_pairs.end(),
                     line_spectral_pairs.begin() + 1,
                     [output_scale](double w) { return w * output_scale; });
    }

    if (!sptk::WriteStream(read_write_point, read_write_size,
                           line_spectral_pairs, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write line spectral pairs";
      sptk::PrintErrorMessage("lspcheck", error_message);
      return 1;
    }
  }

  return 0;
}
