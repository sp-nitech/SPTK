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

#include <cfloat>    // DBL_MAX
#include <fstream>   // std::ifstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream
#include <vector>    // std::vector

#include "Getopt/getoptwin.h"
#include "SPTK/analysis/autocorrelation_analysis.h"
#include "SPTK/conversion/spectrum_to_spectrum.h"
#include "SPTK/math/levinson_durbin_recursion.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum WarningType { kIgnore = 0, kWarn, kExit, kNumWarningTypes };

enum InputFormats {
  kLogAmplitudeSpectrumInDecibels = 0,
  kLogAmplitudeSpectrum,
  kAmplitudeSpectrum,
  kPowerSpectrum,
  kWaveform,
  kNumInputFormats
};

const int kDefaultFrameLength(256);
const int kDefaultNumOrder(25);
const WarningType kDefaultWarningType(kIgnore);
const InputFormats kDefaultInputFormat(kWaveform);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " lpc - linear predictive coding analysis" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       lpc [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : frame length                            (   int)[" << std::setw(5) << std::right << kDefaultFrameLength << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : order of linear predictive coefficients (   int)[" << std::setw(5) << std::right << kDefaultNumOrder    << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -e e  : warning type of unstable index          (   int)[" << std::setw(5) << std::right << kDefaultWarningType << "][ 0 <= e <= 2 ]" << std::endl;  // NOLINT
  *stream << "                 0 (no warning)" << std::endl;
  *stream << "                 1 (output the index to stderr)" << std::endl;
  *stream << "                 2 (output the index to stderr and" << std::endl;
  *stream << "                    exit immediately)" << std::endl;
  *stream << "       -q q  : input format                            (   int)[" << std::setw(5) << std::right << kDefaultInputFormat << "][ 0 <= q <= 4 ]" << std::endl;  // NOLINT
  *stream << "                 0 (20*log|X(z)|)" << std::endl;
  *stream << "                 1 (ln|X(z)|)" << std::endl;
  *stream << "                 2 (|X(z)|)" << std::endl;
  *stream << "                 3 (|X(z)|^2)" << std::endl;
  *stream << "                 4 (windowed waveform)" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       windowed data sequence                          (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       linear predictive coefficients                  (double)" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a lpc [ @e option ] [ @e infile ]
 *
 * - @b -l @e int
 *   - frame length @f$(1 \le L)@f$
 * - @b -m @e int
 *   - order of coefficients @f$(0 \le M)@f$
 * - @b -e @e int
 *   - warning type
 *     \arg @c 0 no warning
 *     \arg @c 1 output index
 *     \arg @c 2 output index and exit immediately
 * - @b -q @e int
 *   - input format
 *     \arg @c 0 amplitude spectrum in dB
 *     \arg @c 1 log amplitude spectrum
 *     \arg @c 2 amplitude spectrum
 *     \arg @c 3 power spectrum
 *     \arg @c 4 windowed waveform
 * - @b infile @e str
 *   - double-type windowed data sequence
 * - @b stdout
 *   - double-type linear predictive coefficients
 *
 * The below example calculates the LPC coefficients of @c data.d.
 *
 * @code{.sh}
 *   frame < data.d | window | lpc -m 20 > data.lpc
 * @endcode
 *
 * This is equivalent to the following line.
 *
 * @code{.sh}
 *   frame < data.d | window | acorr -m 20 | levdur -m 20 > data.lpc
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int frame_length(kDefaultFrameLength);
  int num_order(kDefaultNumOrder);
  WarningType warning_type(kDefaultWarningType);
  InputFormats input_format(kDefaultInputFormat);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:e:q:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &frame_length) ||
            frame_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("lpc", error_message);
          return 1;
        }
        break;
      }
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("lpc", error_message);
          return 1;
        }
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
          sptk::PrintErrorMessage("lpc", error_message);
          return 1;
        }
        warning_type = static_cast<WarningType>(tmp);
        break;
      }
      case 'q': {
        const int min(0);
        const int max(static_cast<int>(kNumInputFormats) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -q option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("lpc", error_message);
          return 1;
        }
        input_format = static_cast<InputFormats>(tmp);
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
    sptk::PrintErrorMessage("lpc", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("lpc", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::SpectrumToSpectrum spectrum_to_spectrum(
      frame_length,
      static_cast<sptk::SpectrumToSpectrum::InputOutputFormats>(input_format),
      sptk::SpectrumToSpectrum::InputOutputFormats::kPowerSpectrum, 0.0,
      -DBL_MAX);
  if (kWaveform != input_format && !spectrum_to_spectrum.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize SpectrumToSpectrum";
    sptk::PrintErrorMessage("lpc", error_message);
    return 1;
  }

  sptk::AutocorrelationAnalysis autocorrelation_analysis(
      frame_length, num_order, kWaveform == input_format);
  sptk::AutocorrelationAnalysis::Buffer buffer_for_analysis;
  if (!autocorrelation_analysis.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize AutocorrelationAnalysis";
    sptk::PrintErrorMessage("lpc", error_message);
    return 1;
  }

  sptk::LevinsonDurbinRecursion levinson_durbin_recursion(num_order);
  sptk::LevinsonDurbinRecursion::Buffer buffer_for_levinson;
  if (!levinson_durbin_recursion.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize LevinsonDurbinRecursion";
    sptk::PrintErrorMessage("lpc", error_message);
    return 1;
  }

  const int input_length(kWaveform == input_format ? frame_length
                                                   : frame_length / 2 + 1);
  const int output_length(num_order + 1);
  std::vector<double> input(input_length);
  std::vector<double> autocorrelation(output_length);
  std::vector<double> linear_predictive_coefficients(output_length);

  for (int frame_index(0);
       sptk::ReadStream(false, 0, 0, input_length, &input, &input_stream, NULL);
       ++frame_index) {
    if (kWaveform != input_format) {
      if (!spectrum_to_spectrum.Run(&input)) {
        std::ostringstream error_message;
        error_message << "Failed to convert spectrum";
        sptk::PrintErrorMessage("lpc", error_message);
        return 1;
      }
    }

    if (!autocorrelation_analysis.Run(input, &autocorrelation,
                                      &buffer_for_analysis)) {
      std::ostringstream error_message;
      error_message << "Failed to obtain autocorrelation";
      sptk::PrintErrorMessage("lpc", error_message);
      return 1;
    }

    bool is_stable(false);
    if (!levinson_durbin_recursion.Run(autocorrelation,
                                       &linear_predictive_coefficients,
                                       &is_stable, &buffer_for_levinson)) {
      std::ostringstream error_message;
      error_message << "Failed to solve autocorrelation normal equations";
      sptk::PrintErrorMessage("lpc", error_message);
      return 1;
    }

    if (!is_stable && kIgnore != warning_type) {
      std::ostringstream error_message;
      error_message << frame_index << "th frame is unstable";
      sptk::PrintErrorMessage("lpc", error_message);
      if (kExit == warning_type) return 1;
    }

    if (!sptk::WriteStream(0, output_length, linear_predictive_coefficients,
                           &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write linear predictive coefficients";
      sptk::PrintErrorMessage("lpc", error_message);
      return 1;
    }
  }

  return 0;
}
