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
#include "SPTK/analysis/second_order_all_pass_mel_cepstral_analysis.h"
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

const int kDefaultNumOrder(25);
const double kDefaultAlpha(0.35);
const double kDefaultTheta(0.0);
const int kDefaultFftLength(256);
const InputFormats kDefaultInputFormat(kWaveform);
const int kDefaultAccuracyFactor(4);
const int kDefaultNumIteration(30);
const double kDefaultConvergenceThreshold(1e-3);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " smcep - mel-cepstral analysis based on 2nd order all-pass function" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       smcep [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -m m  : order of mel-cepstrum               (   int)[" << std::setw(5) << std::right << kDefaultNumOrder             << "][    0 <= m <=     ]" << std::endl;  // NOLINT
  *stream << "       -a a  : all-pass constant                   (double)[" << std::setw(5) << std::right << kDefaultAlpha                << "][ -1.0 <  a <  1.0 ]" << std::endl;  // NOLINT
  *stream << "       -t t  : emphasized frequency                (double)[" << std::setw(5) << std::right << kDefaultTheta                << "][  0.0 <= t <= 1.0 ]" << std::endl;  // NOLINT
  *stream << "       -l l  : frame length (FFT length)           (   int)[" << std::setw(5) << std::right << kDefaultFftLength            << "][    2 <= l <=     ]" << std::endl;  // NOLINT
  *stream << "       -q q  : input format                        (   int)[" << std::setw(5) << std::right << kDefaultInputFormat          << "][    0 <= q <= 4   ]" << std::endl;  // NOLINT
  *stream << "                 0 (20*log|X(z)|)" << std::endl;
  *stream << "                 1 (ln|X(z)|)" << std::endl;
  *stream << "                 2 (|X(z)|)" << std::endl;
  *stream << "                 3 (|X(z)|^2)" << std::endl;
  *stream << "                 4 (windowed waveform)" << std::endl;
  *stream << "     (level 2)" << std::endl;
  *stream << "       -f f  : accuracy factor                     (   int)[" << std::setw(5) << std::right << kDefaultAccuracyFactor       << "][    1 <= f <=     ]" << std::endl;  // NOLINT
  *stream << "       -i i  : maximum number of iterations        (   int)[" << std::setw(5) << std::right << kDefaultNumIteration         << "][    0 <= i <=     ]" << std::endl;  // NOLINT
  *stream << "       -d d  : convergence threshold               (double)[" << std::setw(5) << std::right << kDefaultConvergenceThreshold << "][  0.0 <= d <=     ]" << std::endl;  // NOLINT
  *stream << "       -e e  : small value added to power spectrum (double)[" << std::setw(5) << std::right << "N/A"                        << "][  0.0 <  e <=     ]" << std::endl;  // NOLINT
  *stream << "       -E E  : relative floor in decibels          (double)[" << std::setw(5) << std::right << "N/A"                        << "][      <= E <  0.0 ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       windowed data sequence or spectrum          (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       mel-cepstrum                                (double)" << std::endl;  // NOLINT
  *stream << "  notice:" << std::endl;
  *stream << "       value of l and f must be a power of 2" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a smcep [ @e option ] [ @e infile ]
 *
 * - @b -m @e int
 *   - order of coefficients @f$(0 \le M)@f$
 * - @b -a @e double
 *   - all-pass constant @f$(|\alpha| < 1)@f$
 * - @b -t @e double
 *   - emphasized frequency in @f$\pi@f$ rad @f$(0 \le \theta \le 1.0)@f$
 * - @b -l @e int
 *   - FFT length @f$(2 \le N)@f$
 * - @b -q @e int
 *   - input format
 *     \arg @c 0 amplitude spectrum in dB
 *     \arg @c 1 log amplitude spectrum
 *     \arg @c 2 amplitude spectrum
 *     \arg @c 3 power spectrum
 *     \arg @c 4 windowed waveform
 * - @b -f @e int
 *   - accuracy factor @f$(1 \le F)@f$
 * - @b -i @e int
 *   - number of iterations @f$(0 \le J)@f$
 * - @b -d @e double
 *   - convergence threshold @f$(0 \le \epsilon)@f$
 * - @b -e @e double
 *   - small value added to power spectrum
 * - @b -E @e double
 *   - relative floor in decibels
 * - @b infile @e str
 *   - double-type windowed sequence or spectrum
 * - @b stdout
 *   - double-type mel-cepstral coefficients
 *
 * In the example below, mel-cepstral coefficients are extracted from @c data.d.
 *
 * @code{.sh}
 *   frame < data.d | window | smcep -t 0.12 > data.mcep
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);
  double alpha(kDefaultAlpha);
  double theta(kDefaultTheta);
  int fft_length(kDefaultFftLength);
  int accuracy_factor(kDefaultAccuracyFactor);
  InputFormats input_format(kDefaultInputFormat);
  int num_iteration(kDefaultNumIteration);
  double convergence_threshold(kDefaultConvergenceThreshold);
  double epsilon(0.0);
  double relative_floor_in_decibels(-DBL_MAX);

  for (;;) {
    const int option_char(
        getopt_long(argc, argv, "m:a:t:l:q:f:i:d:e:E:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("smcep", error_message);
          return 1;
        }
        break;
      }
      case 'a': {
        if (!sptk::ConvertStringToDouble(optarg, &alpha) ||
            !sptk::IsValidAlpha(alpha)) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -a option must be in (-1.0, 1.0)";
          sptk::PrintErrorMessage("smcep", error_message);
          return 1;
        }
        break;
      }
      case 't': {
        if (!sptk::ConvertStringToDouble(optarg, &theta) || theta < 0.0 ||
            1.0 < theta) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -t option must be in [0.0, 1.0]";
          sptk::PrintErrorMessage("smcep", error_message);
          return 1;
        }
        break;
      }
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &fft_length)) {
          std::ostringstream error_message;
          error_message << "The argument for the -l option must be an integer";
          sptk::PrintErrorMessage("smcep", error_message);
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
          sptk::PrintErrorMessage("smcep", error_message);
          return 1;
        }
        input_format = static_cast<InputFormats>(tmp);
        break;
      }
      case 'f': {
        if (!sptk::ConvertStringToInteger(optarg, &accuracy_factor) ||
            !sptk::IsPowerOfTwo(accuracy_factor)) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -f option must be a powewr of two";
          sptk::PrintErrorMessage("smcep", error_message);
          return 1;
        }
        break;
      }
      case 'i': {
        if (!sptk::ConvertStringToInteger(optarg, &num_iteration) ||
            num_iteration < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -i option must be a "
                           "non-negative integer";
          sptk::PrintErrorMessage("smcep", error_message);
          return 1;
        }
        break;
      }
      case 'd': {
        if (!sptk::ConvertStringToDouble(optarg, &convergence_threshold) ||
            convergence_threshold < 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -d option must be a non-negative number";
          sptk::PrintErrorMessage("smcep", error_message);
          return 1;
        }
        break;
      }
      case 'e': {
        if (!sptk::ConvertStringToDouble(optarg, &epsilon) || epsilon <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -e option must be a positive number";
          sptk::PrintErrorMessage("smcep", error_message);
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
          sptk::PrintErrorMessage("smcep", error_message);
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
    sptk::PrintErrorMessage("smcep", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("smcep", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::SpectrumToSpectrum spectrum_to_spectrum(
      fft_length,
      static_cast<sptk::SpectrumToSpectrum::InputOutputFormats>(input_format),
      sptk::SpectrumToSpectrum::InputOutputFormats::kPowerSpectrum, epsilon,
      relative_floor_in_decibels);
  if (kWaveform != input_format && !spectrum_to_spectrum.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to set condition for input formatting";
    sptk::PrintErrorMessage("smcep", error_message);
    return 1;
  }

  sptk::WaveformToSpectrum waveform_to_spectrum(
      fft_length, fft_length,
      sptk::SpectrumToSpectrum::InputOutputFormats::kPowerSpectrum, epsilon,
      relative_floor_in_decibels);
  sptk::WaveformToSpectrum::Buffer buffer_for_spectral_analysis;
  if (kWaveform == input_format && !waveform_to_spectrum.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to set condition for spectral analysis";
    sptk::PrintErrorMessage("smcep", error_message);
    return 1;
  }

  sptk::SecondOrderAllPassMelCepstralAnalysis analysis(
      fft_length, num_order, accuracy_factor, alpha, theta * sptk::kPi,
      num_iteration, convergence_threshold);
  sptk::SecondOrderAllPassMelCepstralAnalysis::Buffer
      buffer_for_cepstral_analysis;
  if (!analysis.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to set condition for cepstral analysis";
    sptk::PrintErrorMessage("smcep", error_message);
    return 1;
  }

  const int input_length(kWaveform == input_format ? fft_length
                                                   : fft_length / 2 + 1);
  const int output_length(num_order + 1);
  std::vector<double> input(input_length);
  std::vector<double> processed_input(fft_length / 2 + 1);
  std::vector<double> output(output_length);

  while (sptk::ReadStream(false, 0, 0, input_length, &input, &input_stream,
                          NULL)) {
    if (kWaveform != input_format) {
      if (!spectrum_to_spectrum.Run(input, &processed_input)) {
        std::ostringstream error_message;
        error_message << "Failed to convert spectrum";
        sptk::PrintErrorMessage("smcep", error_message);
        return 1;
      }
    } else {
      if (!waveform_to_spectrum.Run(input, &processed_input,
                                    &buffer_for_spectral_analysis)) {
        std::ostringstream error_message;
        error_message << "Failed to transform waveform to spectrum";
        sptk::PrintErrorMessage("smcep", error_message);
        return 1;
      }
    }

    if (!analysis.Run(processed_input, &output,
                      &buffer_for_cepstral_analysis)) {
      std::ostringstream error_message;
      error_message << "Failed to run mel-cepstral analysis";
      sptk::PrintErrorMessage("smcep", error_message);
      return 1;
    }

    if (!sptk::WriteStream(0, output_length, output, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write mel-cepstrum";
      sptk::PrintErrorMessage("smcep", error_message);
      return 1;
    }
  }

  return 0;
}
