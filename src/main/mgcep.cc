// ----------------------------------------------------------------- //
//             The Speech Signal Processing Toolkit (SPTK)           //
//             developed by SPTK Working Group                       //
//             http://sp-tk.sourceforge.net/                         //
// ----------------------------------------------------------------- //
//                                                                   //
//  Copyright (c) 1984-2007  Tokyo Institute of Technology           //
//                           Interdisciplinary Graduate School of    //
//                           Science and Engineering                 //
//                                                                   //
//                1996-2019  Nagoya Institute of Technology          //
//                           Department of Computer Science          //
//                                                                   //
// All rights reserved.                                              //
//                                                                   //
// Redistribution and use in source and binary forms, with or        //
// without modification, are permitted provided that the following   //
// conditions are met:                                               //
//                                                                   //
// - Redistributions of source code must retain the above copyright  //
//   notice, this list of conditions and the following disclaimer.   //
// - Redistributions in binary form must reproduce the above         //
//   copyright notice, this list of conditions and the following     //
//   disclaimer in the documentation and/or other materials provided //
//   with the distribution.                                          //
// - Neither the name of the SPTK working group nor the names of its //
//   contributors may be used to endorse or promote products derived //
//   from this software without specific prior written permission.   //
//                                                                   //
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            //
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       //
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          //
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          //
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS //
// BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          //
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   //
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     //
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON //
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   //
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    //
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           //
// POSSIBILITY OF SUCH DAMAGE.                                       //
// ----------------------------------------------------------------- //

#include <getopt.h>   // getopt_long
#include <algorithm>  // std::copy, std::transform
#include <cfloat>     // DBL_MAX
#include <cmath>      // std::exp, std::pow
#include <fstream>    // std::ifstream
#include <iomanip>    // std::setw
#include <iostream>   // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>    // std::ostringstream
#include <vector>     // std::vector

#include "SPTK/analyzer/mel_generalized_cepstral_analysis.h"
#include "SPTK/converter/mel_cepstrum_to_mlsa_digital_filter_coefficients.h"
#include "SPTK/converter/waveform_to_spectrum.h"
#include "SPTK/normalizer/generalized_cepstrum_gain_normalization.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum InputFormats {
  kLogAmplitudeSpectrumInDecibels = 0,
  kLogAmplitudeSpectrum,
  kAmplitudeSpectrum,
  kPeriodogram,
  kWaveform,
  kNumInputFormats
};

enum OutputFormats {
  kCepstrum = 0,
  kMlsaFilterCoefficients,
  kGainNormalizedCepstrum,
  kGainNormalizedMlsaFilterCoefficients,
  kNumOutputFormats
};

const int kDefaultNumOrder(25);
const double kDefaultAlpha(0.35);
const double kDefaultGamma(0.0);
const int kDefaultFftLength(256);
const InputFormats kDefaultInputFormat(kWaveform);
const OutputFormats kDefaultOutputFormat(kCepstrum);
const int kDefaultNumIteration(10);
const double kDefaultConvergenceThreshold(1e-3);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " mgcep - mel-generalized cepstral analysis" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       mgcep [ options ] [ infile ] > stdout" << std::endl;  // NOLINT
  *stream << "  options:" << std::endl;
  *stream << "       -m m  : order of mel-generalized cepstrum (   int)[" << std::setw(5) << std::right << kDefaultNumOrder             << "][    0 <= m <=     ]" << std::endl;  // NOLINT
  *stream << "       -a a  : all-pass constant                 (double)[" << std::setw(5) << std::right << kDefaultAlpha                << "][ -1.0 <  a <  1.0 ]" << std::endl;  // NOLINT
  *stream << "       -g g  : gamma                             (double)[" << std::setw(5) << std::right << kDefaultGamma                << "][ -1.0 <= g <= 0.0 ]" << std::endl;  // NOLINT
  *stream << "       -c c  : gamma = -1 / c                    (   int)[" << std::setw(5) << std::right << "N/A"                        << "][    1 <= c <=     ]" << std::endl;  // NOLINT
  *stream << "       -l l  : frame length (FFT length)         (   int)[" << std::setw(5) << std::right << kDefaultFftLength            << "][    2 <= l <=     ]" << std::endl;  // NOLINT
  *stream << "       -q q  : input format                      (   int)[" << std::setw(5) << std::right << kDefaultInputFormat          << "][    0 <= q <= 4   ]" << std::endl;  // NOLINT
  *stream << "                 0 (20*log|X(z)|)" << std::endl;
  *stream << "                 1 (ln|X(z)|)" << std::endl;
  *stream << "                 2 (|X(z)|)" << std::endl;
  *stream << "                 3 (|X(z)|^2)" << std::endl;
  *stream << "                 4 (windowed waveform)" << std::endl;
  *stream << "       -o o  : output format                     (   int)[" << std::setw(5) << std::right << kDefaultOutputFormat         << "][    0 <= o <= 3   ]" << std::endl;  // NOLINT
  *stream << "                 0 (mel-cepstrum)" << std::endl;
  *stream << "                 1 (mlsa filter coefficients)" << std::endl;
  *stream << "                 2 (gain normalized cepstrum)" << std::endl;
  *stream << "                 3 (gain normalized mlsa filter coefficients)" << std::endl;  // NOLINT
  *stream << "     (level 2)" << std::endl;
  *stream << "       -i i  : maximum number of iterations      (   int)[" << std::setw(5) << std::right << kDefaultNumIteration         << "][    0 <= i <=     ]" << std::endl;  // NOLINT
  *stream << "       -d d  : convergence threshold             (double)[" << std::setw(5) << std::right << kDefaultConvergenceThreshold << "][  0.0 <= d <=     ]" << std::endl;  // NOLINT
  *stream << "       -e e  : small value for calculating log   (double)[" << std::setw(5) << std::right << "N/A"                        << "][  0.0 <  e <=     ]" << std::endl;  // NOLINT
  *stream << "       -E E  : relative floor in decibels        (double)[" << std::setw(5) << std::right << "N/A"                        << "][      <= E <  0.0 ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       windowed data sequence or spectrum        (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       mel-generalized cepstrum                  (double)" << std::endl;  // NOLINT
  *stream << "  notice:" << std::endl;
  *stream << "       value of l must be a power of 2" << std::endl;
  *stream << "       -e and -E are valid only if q is 4" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);
  double alpha(kDefaultAlpha);
  double gamma(kDefaultGamma);
  int fft_length(kDefaultFftLength);
  InputFormats input_format(kDefaultInputFormat);
  OutputFormats output_format(kDefaultOutputFormat);
  int num_iteration(kDefaultNumIteration);
  double convergence_threshold(kDefaultConvergenceThreshold);
  double epsilon_for_calculating_logarithms(0.0);
  double relative_floor_in_decibels(-DBL_MAX);

  for (;;) {
    const int option_char(
        getopt_long(argc, argv, "m:a:g:c:l:q:o:i:d:e:E:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("mgcep", error_message);
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
          sptk::PrintErrorMessage("mgcep", error_message);
          return 1;
        }
        break;
      }
      case 'g': {
        if (!sptk::ConvertStringToDouble(optarg, &gamma) ||
            !sptk::IsValidGamma(gamma)) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -g option must be in [-1.0, 0.0]";
          sptk::PrintErrorMessage("mgcep", error_message);
          return 1;
        }
        break;
      }
      case 'c': {
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) || tmp < 1) {
          std::ostringstream error_message;
          error_message << "The argument for the -c option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("mgcep", error_message);
          return 1;
        }
        gamma = -1.0 / tmp;
        break;
      }
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &fft_length)) {
          std::ostringstream error_message;
          error_message << "The argument for the -l option must be an integer";
          sptk::PrintErrorMessage("mgcep", error_message);
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
          sptk::PrintErrorMessage("mgcep", error_message);
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
          sptk::PrintErrorMessage("mgcep", error_message);
          return 1;
        }
        output_format = static_cast<OutputFormats>(tmp);
        break;
      }
      case 'i': {
        if (!sptk::ConvertStringToInteger(optarg, &num_iteration) ||
            num_iteration < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -i option must be a "
                           "non-negative integer";
          sptk::PrintErrorMessage("mgcep", error_message);
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
          sptk::PrintErrorMessage("mgcep", error_message);
          return 1;
        }
        break;
      }
      case 'e': {
        if (!sptk::ConvertStringToDouble(optarg,
                                         &epsilon_for_calculating_logarithms) ||
            epsilon_for_calculating_logarithms <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -e option must be a positive number";
          sptk::PrintErrorMessage("mgcep", error_message);
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
          sptk::PrintErrorMessage("mgcep", error_message);
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

  // get input file
  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("mgcep", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  // open stream
  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("mgcep", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::WaveformToSpectrum waveform_to_spectrum(
      fft_length, fft_length,
      sptk::FilterCoefficientsToSpectrum::OutputFormats::kPowerSpectrum,
      epsilon_for_calculating_logarithms, relative_floor_in_decibels);
  sptk::WaveformToSpectrum::Buffer buffer_for_spectral_analysis;
  if (!waveform_to_spectrum.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to set condition for spectral analysis";
    sptk::PrintErrorMessage("mgcep", error_message);
    return 1;
  }

  sptk::MelGeneralizedCepstralAnalysis analysis(fft_length, num_order, alpha,
                                                gamma, num_iteration,
                                                convergence_threshold);
  sptk::MelGeneralizedCepstralAnalysis::Buffer buffer_for_cepstral_analysis;
  if (!analysis.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to set condition for cepstral analysis";
    sptk::PrintErrorMessage("mgcep", error_message);
    return 1;
  }

  sptk::MelCepstrumToMlsaDigitalFilterCoefficients
      mel_cepstrum_to_mlsa_digital_filter_coefficients(num_order, alpha);
  if (!mel_cepstrum_to_mlsa_digital_filter_coefficients.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to set condition for output formatting";
    sptk::PrintErrorMessage("mgcep", error_message);
    return 1;
  }

  sptk::GeneralizedCepstrumGainNormalization
      generalized_cepstrum_gain_normalization(num_order, gamma);
  if (!generalized_cepstrum_gain_normalization.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to set condition for gain normalization";
    sptk::PrintErrorMessage("mgcep", error_message);
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
    switch (input_format) {
      case kLogAmplitudeSpectrumInDecibels: {
        std::transform(input.begin(), input.end(), processed_input.begin(),
                       [](double x) { return std::pow(10, 0.1 * x); });
        break;
      }
      case kLogAmplitudeSpectrum: {
        std::transform(input.begin(), input.end(), processed_input.begin(),
                       [](double x) { return std::exp(2.0 * x); });
        break;
      }
      case kAmplitudeSpectrum: {
        std::transform(input.begin(), input.end(), processed_input.begin(),
                       [](double x) { return x * x; });
        break;
      }
      case kPeriodogram: {
        std::copy(input.begin(), input.end(), processed_input.begin());
        break;
      }
      case kWaveform: {
        if (!waveform_to_spectrum.Run(input, &processed_input,
                                      &buffer_for_spectral_analysis)) {
          std::ostringstream error_message;
          error_message << "Failed to transform waveform to spectrum";
          sptk::PrintErrorMessage("mgcep", error_message);
          return 1;
        }
        break;
      }
      default: { break; }
    }

    if (!analysis.Run(processed_input, &output,
                      &buffer_for_cepstral_analysis)) {
      std::ostringstream error_message;
      error_message << "Failed to run mel-generalized cepstral analysis";
      sptk::PrintErrorMessage("mgcep", error_message);
      return 1;
    }

    if (0.0 != alpha &&
        (kMlsaFilterCoefficients == output_format ||
         kGainNormalizedMlsaFilterCoefficients == output_format)) {
      if (!mel_cepstrum_to_mlsa_digital_filter_coefficients.Run(&output)) {
        std::ostringstream error_message;
        error_message << "Failed to convert to MLSA filter coefficients";
        sptk::PrintErrorMessage("mgcep", error_message);
        return 1;
      }
    }

    if (kGainNormalizedCepstrum == output_format ||
        kGainNormalizedMlsaFilterCoefficients == output_format) {
      if (!generalized_cepstrum_gain_normalization.Run(&output)) {
        std::ostringstream error_message;
        error_message << "Failed to normalize generalized cepstrum";
        sptk::PrintErrorMessage("mgcep", error_message);
        return 1;
      }
    }

    if (!sptk::WriteStream(0, output_length, output, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write mel-generalized cepstrum";
      sptk::PrintErrorMessage("mgcep", error_message);
      return 1;
    }
  }

  return 0;
}
