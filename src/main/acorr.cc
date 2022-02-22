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
#include <cfloat>     // DBL_MAX
#include <fstream>    // std::ifstream
#include <iomanip>    // std::setw
#include <iostream>   // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>    // std::ostringstream
#include <vector>     // std::vector

#include "Getopt/getoptwin.h"
#include "SPTK/analysis/autocorrelation_analysis.h"
#include "SPTK/conversion/spectrum_to_spectrum.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum InputFormats {
  kLogAmplitudeSpectrumInDecibels = 0,
  kLogAmplitudeSpectrum,
  kAmplitudeSpectrum,
  kPowerSpectrum,
  kWaveform,
  kNumInputFormats
};

enum OutputFormats {
  kAutocorrelation = 0,
  kBiasedAutocorrelation,
  kNormalizedAutocorrelation,
  kNumOutputFormats
};

const int kDefaultFrameLength(256);
const int kDefaultNumOrder(25);
const InputFormats kDefaultInputFormat(kWaveform);
const OutputFormats kDefaultOutputFormat(kAutocorrelation);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " acorr - obtain autocorrelation sequence" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       acorr [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : frame length (FFT length) (   int)[" << std::setw(5) << std::right << kDefaultFrameLength  << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : order of autocorrelation  (   int)[" << std::setw(5) << std::right << kDefaultNumOrder     << "][ 0 <= m <    ]" << std::endl;  // NOLINT
  *stream << "       -q q  : input format              (   int)[" << std::setw(5) << std::right << kDefaultInputFormat  << "][ 0 <= q <= 4 ]" << std::endl;  // NOLINT
  *stream << "                 0 (20*log|X(z)|)" << std::endl;
  *stream << "                 1 (ln|X(z)|)" << std::endl;
  *stream << "                 2 (|X(z)|)" << std::endl;
  *stream << "                 3 (|X(z)|^2)" << std::endl;
  *stream << "                 4 (windowed waveform)" << std::endl;
  *stream << "       -o o  : output format             (   int)[" << std::setw(5) << std::right << kDefaultOutputFormat << "][ 0 <= o <= 2 ]" << std::endl;  // NOLINT
  *stream << "                 0 (autocorrelation)" << std::endl;
  *stream << "                 1 (biased autocorrelation)" << std::endl;
  *stream << "                 2 (normalized autocorrelation)" << std::endl;
  *stream << "       -e e  : small value added to      (double)[" << std::setw(5) << std::right << "N/A"                << "][ 0 <  e <=   ]" << std::endl;  // NOLINT
  *stream << "               power spectrum" << std::endl;
  *stream << "       -E E  : relative floor            (double)[" << std::setw(5) << std::right << "N/A"                << "][   <= E <  0 ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence                     (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       autocorrelation sequence          (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a acorr [ @e option ] [ @e infile ]
 *
 * - @b -l @e int
 *   - frame length @f$(1 \le L)@f$
 * - @b -m @e int
 *   - order of autocorrelation coefficients @f$(0 \le M)@f$
 * - @b -q @e int
 *   - input format
 *     \arg @c 0 amplitude spectrum in dB
 *     \arg @c 1 log amplitude spectrum
 *     \arg @c 2 amplitude spectrum
 *     \arg @c 3 power spectrum
 *     \arg @c 4 windowed waveform
 * - @b -o @e double
 *   - output format
 *     \arg @c 0 autocorrelation
 *     \arg @c 1 biased autocorrelation
 *     \arg @c 2 normalized autocorrelation
 * - @b -e @e double
 *   - small value added to power spectrum
 * - @b -E @e double
 *   - relative floor in decibels
 * - @b infile @e str
 *   - double-type data sequence
 * - @b stdout
 *   - double-type autocorrelation sequence.
 *
 * If `-o 1`, output the biased autocorrelation:
 * @f[
 *   \begin{array}{cccc}
 *     r(0)/L, & r(1)/L, & \ldots, & r(M)/L,
 *   \end{array}
 * @f]
 * where @f$r(m)@f$ is the @f$m@f$-th autocorrelation coefficient.
 * If `-o 2`, output the normalized autocorrelation:
 * @f[
 *   \begin{array}{cccc}
 *     1, & r(1)/r(0), & \ldots, & r(M)/r(0).
 *   \end{array}
 * @f]
 *
 * The below example extracts 10-th order autocorrelation coefficients from
 * windowed waveform.
 *
 * @code{.sh}
 *   x2x +sd data.short | frame | window | acorr -m 10 > data.acr
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int frame_length(kDefaultFrameLength);
  int num_order(kDefaultNumOrder);
  InputFormats input_format(kDefaultInputFormat);
  OutputFormats output_format(kDefaultOutputFormat);
  double epsilon(0.0);
  double relative_floor_in_decibels(-DBL_MAX);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:q:o:e:E:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &frame_length) ||
            frame_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("acorr", error_message);
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
          sptk::PrintErrorMessage("acorr", error_message);
          return 1;
        }
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
          sptk::PrintErrorMessage("acorr", error_message);
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
          sptk::PrintErrorMessage("acorr", error_message);
          return 1;
        }
        output_format = static_cast<OutputFormats>(tmp);
        break;
      }
      case 'e': {
        if (!sptk::ConvertStringToDouble(optarg, &epsilon) || epsilon <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -e option must be a positive number";
          sptk::PrintErrorMessage("acorr", error_message);
          return 1;
        }
        break;
      }
      case 'E': {
        if (!sptk::ConvertStringToDouble(optarg, &relative_floor_in_decibels) ||
            0.0 <= relative_floor_in_decibels) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -E option must be a negative number";
          sptk::PrintErrorMessage("acorr", error_message);
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

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("acorr", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("acorr", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::SpectrumToSpectrum spectrum_to_spectrum(
      frame_length,
      static_cast<sptk::SpectrumToSpectrum::InputOutputFormats>(input_format),
      sptk::SpectrumToSpectrum::InputOutputFormats::kPowerSpectrum, epsilon,
      relative_floor_in_decibels);
  if (kWaveform != input_format && !spectrum_to_spectrum.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize SpectrumToSpectrum";
    sptk::PrintErrorMessage("acorr", error_message);
    return 1;
  }

  sptk::AutocorrelationAnalysis analysis(frame_length, num_order,
                                         kWaveform == input_format);
  sptk::AutocorrelationAnalysis::Buffer buffer;
  if (!analysis.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize AutocorrelationAnalysis";
    sptk::PrintErrorMessage("acorr", error_message);
    return 1;
  }

  const int input_length(kWaveform == input_format ? frame_length
                                                   : frame_length / 2 + 1);
  const int output_length(num_order + 1);
  std::vector<double> input(input_length);
  std::vector<double> autocorrelation(output_length);

  while (sptk::ReadStream(false, 0, 0, input_length, &input, &input_stream,
                          NULL)) {
    if (kWaveform != input_format) {
      if (!spectrum_to_spectrum.Run(&input)) {
        std::ostringstream error_message;
        error_message << "Failed to convert spectrum";
        sptk::PrintErrorMessage("acorr", error_message);
        return 1;
      }
    }

    if (!analysis.Run(input, &autocorrelation, &buffer)) {
      std::ostringstream error_message;
      error_message << "Failed to calculate autocorrelation";
      sptk::PrintErrorMessage("acorr", error_message);
      return 1;
    }

    if (kBiasedAutocorrelation == output_format) {
      const double z(1.0 / frame_length);
      std::transform(autocorrelation.begin(), autocorrelation.end(),
                     autocorrelation.begin(), [z](double r) { return r * z; });
    } else if (kNormalizedAutocorrelation == output_format) {
      if (0.0 == autocorrelation[0]) {
        std::ostringstream error_message;
        error_message << "Failed to normalize autocorrelation";
        sptk::PrintErrorMessage("acorr", error_message);
        return 1;
      }

      const double z(1.0 / autocorrelation[0]);
      std::transform(autocorrelation.begin(), autocorrelation.end(),
                     autocorrelation.begin(), [z](double r) { return r * z; });
    }

    if (!sptk::WriteStream(0, output_length, autocorrelation, &std::cout,
                           NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write autocorrelation";
      sptk::PrintErrorMessage("acorr", error_message);
      return 1;
    }
  }

  return 0;
}
