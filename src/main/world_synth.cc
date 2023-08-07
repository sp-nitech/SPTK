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
#include <cmath>      // std::exp
#include <fstream>    // std::ifstream
#include <iomanip>    // std::setw
#include <iostream>   // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>    // std::ostringstream
#include <vector>     // std::vector

#include "GETOPT/ya_getopt.h"
#include "SPTK/conversion/spectrum_to_spectrum.h"
#include "SPTK/filter/world_synthesis.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum PitchFormats { kPitch = 0, kF0, kLogF0, kNumPitchFormats };

enum AperiodicityFormats {
  kAperiodicity = 0,
  kPeriodicity,
  kAperiodicityOverPeriodicity,
  kPeriodicityOverAperiodicity,
  kNumAperiodicityFormats
};

const int kDefaultFftLength(1024);
const int kDefaultFrameShift(80);
const double kDefaultSamplingRate(16.0);
const PitchFormats kDefaultPitchFormat(kPitch);
const sptk::SpectrumToSpectrum::InputOutputFormats kDefaultSpectrumFormat(
    sptk::SpectrumToSpectrum::kLogAmplitudeSpectrumInDecibels);
const AperiodicityFormats kDefaultAperiodicityFormat(kAperiodicity);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " world_synth - WORLD synthesizer" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       world_synth [ options ] spfile apfile [ f0file ] > stdout" << std::endl;  // NOLINT
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : FFT length          (   int)[" << std::setw(5) << std::right << kDefaultFftLength          << "][ 512 <= l <=      ]" << std::endl;  // NOLINT
  *stream << "       -p p  : frame shift [point] (   int)[" << std::setw(5) << std::right << kDefaultFrameShift         << "][   1 <= p <=      ]" << std::endl;  // NOLINT
  *stream << "       -s s  : sampling rate [kHz] (double)[" << std::setw(5) << std::right << kDefaultSamplingRate       << "][ 8.0 <= s <= 98.0 ]" << std::endl;  // NOLINT
  *stream << "       -F F  : pitch format        (   int)[" << std::setw(5) << std::right << kDefaultPitchFormat        << "][   0 <= F <= 2    ]" << std::endl;  // NOLINT
  *stream << "                 0 (Fs/F0)" << std::endl;
  *stream << "                 1 (F0)" << std::endl;
  *stream << "                 2 (log F0)" << std::endl;
  *stream << "       -S S  : spectrum format     (   int)[" << std::setw(5) << std::right << kDefaultSpectrumFormat     << "][   0 <= S <= 3    ]" << std::endl;  // NOLINT
  *stream << "                 0 (20*log|H(z)|)" << std::endl;
  *stream << "                 1 (ln|H(z)|)" << std::endl;
  *stream << "                 2 (|H(z)|)" << std::endl;
  *stream << "                 3 (|H(z)|^2)" << std::endl;
  *stream << "       -A A  : aperiodicity format (   int)[" << std::setw(5) << std::right << kDefaultAperiodicityFormat << "][   0 <= A <= 3    ]" << std::endl;  // NOLINT
  *stream << "                 0 (Ha)" << std::endl;
  *stream << "                 1 (Hp)" << std::endl;
  *stream << "                 2 (Ha/Hp)" << std::endl;
  *stream << "                 3 (Hp/Ha)" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  spfile:" << std::endl;
  *stream << "       spectrum                    (double)" << std::endl;
  *stream << "  apfile:" << std::endl;
  *stream << "       aperiodicity                (double)" << std::endl;
  *stream << "  f0file:" << std::endl;
  *stream << "       pitch, not excitation       (double)" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       waveform                    (double)" << std::endl;
  *stream << "  notice:" << std::endl;
  *stream << "       value of l must be a power of 2" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a world_synth [ @e option ] @e spfile @e apfile [ @e f0file ]
 *
 * - @b -l @e int
 *   - FFT length
 * - @b -p @e int
 *   - frame shift [point] @f$(1 \le P)@f$
 * - @b -s @e double
 *   - sampling rate [kHz] @f$(8 < F_s \le 98)@f$
 * - @b -F @e int
 *   - pitch format
 *     @arg @c 0 pitch @f$(F_s / F_0)@f$
 *     @arg @c 1 F0
 *     @arg @c 2 log F0
 * - @b -S @e int
 *   - spectrum format
 *     \arg @c 0 @f$20 \log_{10} |H(z)|@f$
 *     \arg @c 1 @f$\log |H(z)|@f$
 *     \arg @c 2 @f$|H(z)|@f$
 *     \arg @c 3 @f$|H(z)|^2@f$
 * - @b -A @e int
 *   - aperiodicity format
 *     @arg @c 0 Ha
 *     @arg @c 1 Hp
 *     @arg @c 2 Ha/Hp
 *     @arg @c 3 Hp/Ha
 * - @b spfile @e str
 *   - double-type spectrum
 * - @b apfile @e str
 *   - double-type aperiodicity
 * - @b f0file @e str
 *   - double-type pitch, not exicitaiton
 * - @b stdout
 *   - double-type waveform
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int fft_length(kDefaultFftLength);
  int frame_shift(kDefaultFrameShift);
  double sampling_rate(kDefaultSamplingRate);
  PitchFormats pitch_format(kDefaultPitchFormat);
  sptk::SpectrumToSpectrum::InputOutputFormats spectrum_format(
      kDefaultSpectrumFormat);
  AperiodicityFormats aperiodicity_format(kDefaultAperiodicityFormat);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:p:s:F:S:A:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &fft_length) ||
            fft_length < 512) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be at least 512";
          sptk::PrintErrorMessage("world_synth", error_message);
          return 1;
        }
        break;
      }
      case 'p': {
        if (!sptk::ConvertStringToInteger(optarg, &frame_shift) ||
            frame_shift <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -p option must be a positive integer";
          sptk::PrintErrorMessage("world_synth", error_message);
          return 1;
        }
        break;
      }
      case 's': {
        const double min(8.0);
        const double max(98.0);
        if (!sptk::ConvertStringToDouble(optarg, &sampling_rate) ||
            !sptk::IsInRange(sampling_rate, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -s option must be in a number "
                        << "in the interval [" << min << ", " << max << "]";
          sptk::PrintErrorMessage("world_synth", error_message);
          return 1;
        }
        break;
      }
      case 'F': {
        const int min(0);
        const int max(static_cast<int>(kNumPitchFormats) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -F option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("world_synth", error_message);
          return 1;
        }
        pitch_format = static_cast<PitchFormats>(tmp);
        break;
      }
      case 'S': {
        const int min(0);
        const int max(
            static_cast<int>(sptk::SpectrumToSpectrum::kNumInputOutputFormats) -
            1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -S option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("world_synth", error_message);
          return 1;
        }
        spectrum_format =
            static_cast<sptk::SpectrumToSpectrum::InputOutputFormats>(tmp);
        break;
      }
      case 'A': {
        const int min(0);
        const int max(static_cast<int>(kNumAperiodicityFormats) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -A option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("world_synth", error_message);
          return 1;
        }
        aperiodicity_format = static_cast<AperiodicityFormats>(tmp);
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

  const char* spectrum_file;
  const char* aperiodicity_file;
  const char* f0_file;
  const int num_input_files(argc - optind);
  if (3 == num_input_files) {
    spectrum_file = argv[argc - 3];
    aperiodicity_file = argv[argc - 2];
    f0_file = argv[argc - 1];
  } else if (2 == num_input_files) {
    spectrum_file = argv[argc - 2];
    aperiodicity_file = argv[argc - 1];
    f0_file = NULL;
  } else {
    std::ostringstream error_message;
    error_message
        << "Just three input files, spfile, apfile, and f0file are required";
    sptk::PrintErrorMessage("world_synth", error_message);
    return 1;
  }

  if (!sptk::SetBinaryMode()) {
    std::ostringstream error_message;
    error_message << "Cannot set translation mode";
    sptk::PrintErrorMessage("world_synth", error_message);
    return 1;
  }
  const int spectrum_size(fft_length / 2 + 1);
  const double sampling_rate_in_hz(1000.0 * sampling_rate);

  std::vector<double> f0;
  {
    std::ifstream ifs;
    if (NULL != f0_file) {
      ifs.open(f0_file, std::ios::in | std::ios::binary);
      if (ifs.fail()) {
        std::ostringstream error_message;
        error_message << "Cannot open file " << f0_file;
        sptk::PrintErrorMessage("world_synth", error_message);
        return 1;
      }
    }
    std::istream& input_stream(ifs.is_open() ? ifs : std::cin);

    double tmp;
    while (sptk::ReadStream(&tmp, &input_stream)) {
      f0.push_back(tmp);
    }

    switch (pitch_format) {
      case kPitch: {
        std::transform(f0.begin(), f0.end(), f0.begin(),
                       [sampling_rate_in_hz](double x) {
                         return (0.0 == x) ? 0.0 : sampling_rate_in_hz / x;
                       });
        break;
      }
      case kF0: {
        // nothing to do
        break;
      }
      case kLogF0: {
        std::transform(f0.begin(), f0.end(), f0.begin(), [](double x) {
          return (sptk::kLogZero == x) ? 0.0 : std::exp(x);
        });
        break;
      }
      default: {
        break;
      }
    }
  }
  if (f0.empty()) return 0;

  std::vector<std::vector<double>> spectrum;
  spectrum.reserve(f0.size());
  {
    sptk::SpectrumToSpectrum spectrum_to_spectrum(
        fft_length,
        static_cast<sptk::SpectrumToSpectrum::InputOutputFormats>(
            spectrum_format),
        sptk::SpectrumToSpectrum::InputOutputFormats::kPowerSpectrum);
    if (!spectrum_to_spectrum.IsValid()) {
      std::ostringstream error_message;
      error_message << "FFT length must be a power of 2";
      sptk::PrintErrorMessage("world_synth", error_message);
      return 1;
    }

    std::ifstream ifs;
    ifs.open(spectrum_file, std::ios::in | std::ios::binary);
    if (ifs.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << spectrum_file;
      sptk::PrintErrorMessage("world_synth", error_message);
      return 1;
    }
    std::istream& input_stream(ifs);

    std::vector<double> tmp(spectrum_size);
    while (sptk::ReadStream(false, 0, 0, spectrum_size, &tmp, &input_stream,
                            NULL)) {
      if (!spectrum_to_spectrum.Run(&tmp)) {
        std::ostringstream error_message;
        error_message << "Failed to convert spectrum";
        sptk::PrintErrorMessage("world_synth", error_message);
        return 1;
      }
      spectrum.push_back(tmp);
    }
  }

  std::vector<std::vector<double>> aperiodicity;
  aperiodicity.reserve(f0.size());
  {
    std::ifstream ifs;
    ifs.open(aperiodicity_file, std::ios::in | std::ios::binary);
    if (ifs.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << aperiodicity_file;
      sptk::PrintErrorMessage("world_synth", error_message);
      return 1;
    }
    std::istream& input_stream(ifs);

    std::vector<double> tmp(spectrum_size);
    while (sptk::ReadStream(false, 0, 0, spectrum_size, &tmp, &input_stream,
                            NULL)) {
      switch (aperiodicity_format) {
        case kAperiodicity: {
          // nothing to do
          break;
        }
        case kPeriodicity: {
          std::transform(tmp.begin(), tmp.end(), tmp.begin(),
                         [](double p) { return 1.0 - p; });
          break;
        }
        case kAperiodicityOverPeriodicity: {
          std::transform(tmp.begin(), tmp.end(), tmp.begin(),
                         [](double ap) { return ap / (1.0 + ap); });
          break;
        }
        case kPeriodicityOverAperiodicity: {
          std::transform(tmp.begin(), tmp.end(), tmp.begin(),
                         [](double pa) { return 1.0 / (1.0 + pa); });
          break;
        }
        default: {
          break;
        }
      }
      aperiodicity.push_back(tmp);
    }
  }

  sptk::WorldSynthesis world_synthesis(fft_length, frame_shift,
                                       sampling_rate_in_hz);
  if (!world_synthesis.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize WorldSynthesis";
    sptk::PrintErrorMessage("world_synth", error_message);
    return 1;
  }

  std::vector<double> waveform;
  if (!world_synthesis.Run(f0, spectrum, aperiodicity, &waveform)) {
    std::ostringstream error_message;
    error_message << "Failed to synthesize waveform";
    sptk::PrintErrorMessage("world_synth", error_message);
    return 1;
  }

  if (!sptk::WriteStream(0, static_cast<int>(waveform.size()), waveform,
                         &std::cout, NULL)) {
    std::ostringstream error_message;
    error_message << "Failed to write waveform";
    sptk::PrintErrorMessage("world_synth", error_message);
    return 1;
  }

  return 0;
}
