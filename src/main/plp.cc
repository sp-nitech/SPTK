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

#include "GETOPT/ya_getopt.h"
#include "SPTK/analysis/perceptual_linear_predictive_coefficients_analysis.h"
#include "SPTK/conversion/spectrum_to_spectrum.h"
#include "SPTK/conversion/waveform_to_spectrum.h"
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
  kPlp = 0,
  kPlpAndEnergy,
  kPlpAndC0,
  kPlpAndC0AndEnergy,
  kNumOutputFormats
};

const int kDefaultNumChannel(20);
const int kDefaultNumOrder(12);
const int kDefaultFftLength(256);
const int kDefaultLifteringCoefficient(22);
const double kDefaultSamplingRate(16.0);
const double kDefaultLowestFrequency(0.0);
const double kDefaultCompressionFactor(0.33);
const InputFormats kDefaultInputFormat(kWaveform);
const OutputFormats kDefaultOutputFormat(kPlp);
const double kDefaultFloor(1.0);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " plp - perceptual linear predictive coefficients analysis" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       plp [ options ] [ infile ] > stdout" << std::endl;  // NOLINT
  *stream << "  options:" << std::endl;
  *stream << "       -n n  : number of channels              (   int)[" << std::setw(5) << std::right << kDefaultNumChannel           << "][   1 <= n <=       ]" << std::endl;  // NOLINT
  *stream << "       -m m  : order of cepstrum               (   int)[" << std::setw(5) << std::right << kDefaultNumOrder             << "][   1 <= m <  n     ]" << std::endl;  // NOLINT
  *stream << "       -l l  : frame length (FFT length)       (   int)[" << std::setw(5) << std::right << kDefaultFftLength            << "][   2 <= l <=       ]" << std::endl;  // NOLINT
  *stream << "       -c c  : liftering coefficient           (   int)[" << std::setw(5) << std::right << kDefaultLifteringCoefficient << "][   1 <= c <        ]" << std::endl;  // NOLINT
  *stream << "       -f f  : amplitude compression factor    (double)[" << std::setw(5) << std::right << kDefaultCompressionFactor    << "][ 0.0 <  f <=       ]" << std::endl;  // NOLINT
  *stream << "       -s s  : sampling rate [kHz]             (double)[" << std::setw(5) << std::right << kDefaultSamplingRate         << "][ 0.0 <  s <=       ]" << std::endl;  // NOLINT
  *stream << "       -L L  : lowest frequency [Hz]           (double)[" << std::setw(5) << std::right << kDefaultLowestFrequency      << "][ 0.0 <= L <  H     ]" << std::endl;  // NOLINT
  *stream << "       -H H  : highest frequency [Hz]          (double)[" << std::setw(5) << std::right << "500*s"                      << "][   L <  H <= 500*s ]" << std::endl;  // NOLINT
  *stream << "       -q q  : input format                    (   int)[" << std::setw(5) << std::right << kDefaultInputFormat          << "][   0 <= q <= 4     ]" << std::endl;  // NOLINT
  *stream << "                 0 (20*log|X(z)|)" << std::endl;
  *stream << "                 1 (ln|X(z)|)" << std::endl;
  *stream << "                 2 (|X(z)|)" << std::endl;
  *stream << "                 3 (|X(z)|^2)" << std::endl;
  *stream << "                 4 (windowed waveform)" << std::endl;
  *stream << "       -o o  : output format                   (   int)[" << std::setw(5) << std::right << kDefaultOutputFormat         << "][   0 <= o <= 3     ]" << std::endl;  // NOLINT
  *stream << "                 0 (plp)" << std::endl;
  *stream << "                 1 (plp and energy)" << std::endl;
  *stream << "                 2 (plp and c0)" << std::endl;
  *stream << "                 3 (plp, c0, and energy)" << std::endl;
  *stream << "       -e e  : floor of raw filter-bank output (double)[" << std::setw(5) << std::right << kDefaultFloor                << "][ 0.0 <  e <=       ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       windowed data sequence or spectrum      (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       plp                                     (double)" << std::endl;  // NOLINT
  *stream << "  notice:" << std::endl;
  *stream << "       value of l must be a power of 2" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a plp [ @e option ] [ @e infile ]
 *
 * - @b -n @e int
 *   - number of channels @f$(1 \le C)@f$
 * - @b -m @e int
 *   - order of coeffcients @f$(1 \le M)@f$
 * - @b -l @e int
 *   - FFT length @f$(2 \le N)@f$
 * - @b -c @e int
 *   - liftering parameter @f$(1 \le L)@f$
 * - @b -f @e double
 *   - compression factor @f$(0 < f)@f$
 * - @b -s @e double
 *   - sampling rate in kHz @f$(0 < F_s)@f$
 * - @b -L @e double
 *   - lowest frequency in Hz @f$(0 \le F_l < F_h)@f$
 * - @b -H @e double
 *   - highest frequency in Hz @f$(F_l < F_h \le 500F_s)@f$
 * - @b -q @e int
 *   - input format
 *     @arg @c 0 amplitude spectrum in dB
 *     @arg @c 1 log amplitude spectrum
 *     @arg @c 2 amplitude spectrum
 *     @arg @c 3 power spectrum
 *     @arg @c 4 windowed waveform
 * - @b -o @e int
 *   - output format
 *     @arg @c 0 PLP
 *     @arg @c 1 PLP and energy
 *     @arg @c 2 PLP and C0
 *     @arg @c 3 PLP, C0, and energy
 * - @b -e @e double
 *   - floor value of raw filter-bank output @f$(0 < \epsilon)@f$
 * - @b infile @e str
 *   - double-type windowed sequence or spectrum
 * - @b stdout
 *   - double-type PLP features
 *
 * The below example extracts the 12-th order PLP from @c data.short. The
 * analysis condition is that: frame length is 10 ms, frame shift is 25 ms,
 * and sampling rate is 16 kHz. A pre-emphais filter and the hamming window
 * are applied to the input signal.
 *
 * @code{.sh}
 *   x2x +sd data.short |
 *     frame -l 400 -p 160 -n 1 |
 *     dfs -b 1 -0.97 |
 *     window -l 400 -L 512 -w 1 -n 0 |
 *     plp -l 512 -n 40 -c 22 -m 12 -L 64 -H 4000 -f 0.33 -o 2 > data.plp
 * @endcode
 *
 * The corresponding HTK config file is shown as below.
 *
 * @code
 *   SOURCEFORMAT = NOHEAD
 *   SOURCEKIND   = WAVEFORM
 *   SOURCERATE   = 625.0
 *   TARGETKIND   = PLP_0
 *   TARGETRATE   = 100000.0
 *   WINDOWSIZE   = 250000.0
 *   USEHAMMING   = T
 *   USEPOWER     = T
 *   RAWENERGY    = F
 *   ENORMALIZE   = F
 *   PREEMCOEF    = 0.97
 *   COMPRESSFACT = 0.33
 *   NUMCHANS     = 40
 *   CEPLIFTER    = 22
 *   NUMCEPS      = 12
 *   LOFREQ       = 64
 *   HIFREQ       = 4000
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_channel(kDefaultNumChannel);
  int num_order(kDefaultNumOrder);
  int fft_length(kDefaultFftLength);
  int liftering_coefficient(kDefaultLifteringCoefficient);
  double compression_factor(kDefaultCompressionFactor);
  double sampling_rate(kDefaultSamplingRate);
  double lowest_frequency(kDefaultLowestFrequency);
  double highest_frequency(0.0);
  bool is_highest_frequency_specified(false);
  InputFormats input_format(kDefaultInputFormat);
  OutputFormats output_format(kDefaultOutputFormat);
  double floor(kDefaultFloor);

  for (;;) {
    const int option_char(
        getopt_long(argc, argv, "n:m:l:c:f:s:L:H:q:o:e:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'n': {
        if (!sptk::ConvertStringToInteger(optarg, &num_channel) ||
            num_channel <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -n option must be a positive integer";
          sptk::PrintErrorMessage("plp", error_message);
          return 1;
        }
        break;
      }
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -m option must be a positive integer";
          sptk::PrintErrorMessage("plp", error_message);
          return 1;
        }
        break;
      }
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &fft_length)) {
          std::ostringstream error_message;
          error_message << "The argument for the -l option must be an integer";
          sptk::PrintErrorMessage("plp", error_message);
          return 1;
        }
        break;
      }
      case 'c': {
        if (!sptk::ConvertStringToInteger(optarg, &liftering_coefficient) ||
            liftering_coefficient <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -c option must be a positive integer";
          sptk::PrintErrorMessage("plp", error_message);
          return 1;
        }
        break;
      }
      case 'f': {
        if (!sptk::ConvertStringToDouble(optarg, &compression_factor) ||
            compression_factor <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -f option must be a positive number";
          sptk::PrintErrorMessage("plp", error_message);
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
          sptk::PrintErrorMessage("plp", error_message);
          return 1;
        }
        break;
      }
      case 'L': {
        if (!sptk::ConvertStringToDouble(optarg, &lowest_frequency) ||
            lowest_frequency < 0.0) {
          std::ostringstream error_message;
          error_message << "The argument for the -L option must be a "
                        << "non-negative number";
          sptk::PrintErrorMessage("plp", error_message);
          return 1;
        }
        break;
      }
      case 'H': {
        if (!sptk::ConvertStringToDouble(optarg, &highest_frequency) ||
            highest_frequency <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -H option must be a positive number";
          sptk::PrintErrorMessage("plp", error_message);
          return 1;
        }
        is_highest_frequency_specified = true;
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
          sptk::PrintErrorMessage("plp", error_message);
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
          sptk::PrintErrorMessage("plp", error_message);
          return 1;
        }
        output_format = static_cast<OutputFormats>(tmp);
        break;
      }
      case 'e': {
        if (!sptk::ConvertStringToDouble(optarg, &floor) || floor <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -e option must be a positive number";
          sptk::PrintErrorMessage("plp", error_message);
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
  if (!is_highest_frequency_specified) {
    highest_frequency = 0.5 * sampling_rate_in_hz;
  } else if (0.5 * sampling_rate_in_hz < highest_frequency) {
    std::ostringstream error_message;
    error_message
        << "Highest frequency must be less than or equal to Nyquist frequency";
    sptk::PrintErrorMessage("plp", error_message);
    return 1;
  }

  if (highest_frequency <= lowest_frequency) {
    std::ostringstream error_message;
    error_message << "Lowest frequency must be less than highest one";
    sptk::PrintErrorMessage("plp", error_message);
    return 1;
  }

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("plp", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  if (!sptk::SetBinaryMode()) {
    std::ostringstream error_message;
    error_message << "Cannot set translation mode";
    sptk::PrintErrorMessage("plp", error_message);
    return 1;
  }

  std::ifstream ifs;
  if (NULL != input_file) {
    ifs.open(input_file, std::ios::in | std::ios::binary);
    if (ifs.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << input_file;
      sptk::PrintErrorMessage("plp", error_message);
      return 1;
    }
  }
  std::istream& input_stream(ifs.is_open() ? ifs : std::cin);

  sptk::SpectrumToSpectrum spectrum_to_spectrum(
      fft_length,
      static_cast<sptk::SpectrumToSpectrum::InputOutputFormats>(input_format),
      sptk::SpectrumToSpectrum::InputOutputFormats::kPowerSpectrum);
  if (kWaveform != input_format && !spectrum_to_spectrum.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to set condition for input formatting";
    sptk::PrintErrorMessage("plp", error_message);
    return 1;
  }

  sptk::WaveformToSpectrum waveform_to_spectrum(
      fft_length, fft_length,
      sptk::SpectrumToSpectrum::InputOutputFormats::kPowerSpectrum);
  sptk::WaveformToSpectrum::Buffer buffer_for_spectral_analysis;
  if (kWaveform == input_format && !waveform_to_spectrum.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to set condition for spectral analysis";
    sptk::PrintErrorMessage("plp", error_message);
    return 1;
  }

  sptk::PerceptualLinearPredictiveCoefficientsAnalysis analysis(
      fft_length, num_channel, num_order, liftering_coefficient,
      compression_factor, sampling_rate_in_hz, lowest_frequency,
      highest_frequency, floor);
  sptk::PerceptualLinearPredictiveCoefficientsAnalysis::Buffer
      buffer_for_plp_analysis;
  if (!analysis.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to set condition for PLP analysis";
    sptk::PrintErrorMessage("plp", error_message);
    return 1;
  }

  const int input_length(kWaveform == input_format ? fft_length
                                                   : fft_length / 2 + 1);
  const int output_length(num_order);
  std::vector<double> input(input_length);
  std::vector<double> processed_input(fft_length / 2 + 1);
  std::vector<double> output(output_length);
  double energy(0.0);

  while (sptk::ReadStream(false, 0, 0, input_length, &input, &input_stream,
                          NULL)) {
    if (kWaveform != input_format) {
      if (!spectrum_to_spectrum.Run(input, &processed_input)) {
        std::ostringstream error_message;
        error_message << "Failed to convert spectrum";
        sptk::PrintErrorMessage("plp", error_message);
        return 1;
      }
    } else {
      if (!waveform_to_spectrum.Run(input, &processed_input,
                                    &buffer_for_spectral_analysis)) {
        std::ostringstream error_message;
        error_message << "Failed to transform waveform to spectrum";
        sptk::PrintErrorMessage("plp", error_message);
        return 1;
      }
    }

    if (!analysis.Run(processed_input, &output,
                      (kPlpAndEnergy == output_format ||
                       kPlpAndC0AndEnergy == output_format)
                          ? &energy
                          : NULL,
                      &buffer_for_plp_analysis)) {
      std::ostringstream error_message;
      error_message << "Failed to run PLP analysis";
      sptk::PrintErrorMessage("plp", error_message);
      return 1;
    }

    if (!sptk::WriteStream(1, output_length, output, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write PLP";
      sptk::PrintErrorMessage("plp", error_message);
      return 1;
    }

    if (kPlpAndC0 == output_format || kPlpAndC0AndEnergy == output_format) {
      if (!sptk::WriteStream(output[0], &std::cout)) {
        std::ostringstream error_message;
        error_message << "Failed to write c0";
        sptk::PrintErrorMessage("plp", error_message);
        return 1;
      }
    }

    if (kPlpAndEnergy == output_format || kPlpAndC0AndEnergy == output_format) {
      if (!sptk::WriteStream(energy, &std::cout)) {
        std::ostringstream error_message;
        error_message << "Failed to write energy";
        sptk::PrintErrorMessage("plp", error_message);
        return 1;
      }
    }
  }

  return 0;
}
