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

#include "Getopt/getoptwin.h"
#include "SPTK/conversion/mel_generalized_line_spectral_pairs_to_spectrum.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum InputGainType {
  kLinearGain = 0,
  kLogGain,
  kWithoutGain,
  kNumInputGainTypes
};

enum InputFormats {
  kFrequencyInRadians = 0,
  kFrequencyInCycles,
  kFrequencyInkHz,
  kFrequencyInHz,
  kNumInputFormats
};

enum OutputFormats {
  kLogAmplitudeSpectrumInDecibels = 0,
  kLogAmplitudeSpectrum,
  kAmplitudeSpectrum,
  kPowerSpectrum,
  kNumOutputFormats
};

const int kDefaultNumOrder(25);
const double kDefaultAlpha(0.0);
const double kDefaultGamma(-1.0);
const int kDefaultFftLength(256);
const double kDefaultSamplingRate(10.0);
const InputGainType kDefaultInputGainType(kLinearGain);
const InputFormats kDefaultInputFormat(kFrequencyInRadians);
const OutputFormats kDefaultOutputFormat(kLogAmplitudeSpectrumInDecibels);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " mglsp2sp - transform mel-generalized line spectral pairs to spectrum" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       mglsp2sp [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -m m  : order of mel-generalized line spectral pairs          (   int)[" << std::setw(5) << std::right << kDefaultNumOrder      << "][    0 <= m <=     ]" << std::endl;  // NOLINT
  *stream << "       -a a  : alpha of mel-generalized line spectral pairs          (double)[" << std::setw(5) << std::right << kDefaultAlpha         << "][ -1.0 <  a <  1.0 ]" << std::endl;  // NOLINT
  *stream << "       -g g  : gamma of mel-generalized line spectral pairs          (double)[" << std::setw(5) << std::right << kDefaultGamma         << "][ -1.0 <= g <  0.0 ]" << std::endl;  // NOLINT
  *stream << "       -c c  : gamma of mel-generalized line spectral pairs = -1 / c (   int)[" << std::setw(5) << std::right << "N/A"                 << "][    1 <= c <=     ]" << std::endl;  // NOLINT
  *stream << "       -l l  : fft length                                            (   int)[" << std::setw(5) << std::right << kDefaultFftLength     << "][    1 <= l <=     ]" << std::endl;  // NOLINT
  *stream << "       -s s  : sampling rate                                         (double)[" << std::setw(5) << std::right << kDefaultSamplingRate  << "][  0.0 <  s <=     ]" << std::endl;  // NOLINT
  *stream << "       -k k  : input gain type                                       (   int)[" << std::setw(5) << std::right << kDefaultInputGainType << "][    0 <= k <= 2   ]" << std::endl;  // NOLINT
  *stream << "                 0 (linear gain)" << std::endl;
  *stream << "                 1 (log gain)" << std::endl;
  *stream << "                 2 (without gain)" << std::endl;
  *stream << "       -q q  : input format                                          (   int)[" << std::setw(5) << std::right << kDefaultInputFormat   << "][    0 <= q <= 3   ]" << std::endl;  // NOLINT
  *stream << "                 0 (frequency [rad])" << std::endl;
  *stream << "                 1 (frequency [pi rad])" << std::endl;
  *stream << "                 2 (frequency [kHz])" << std::endl;
  *stream << "                 3 (frequency [Hz])" << std::endl;
  *stream << "       -o o  : output format                                         (   int)[" << std::setw(5) << std::right << kDefaultOutputFormat  << "][    0 <= o <= 3   ]" << std::endl;  // NOLINT
  *stream << "                 0 (20*log|H(z)|)" << std::endl;
  *stream << "                 1 (ln|H(z)|)" << std::endl;
  *stream << "                 2 (|H(z)|)" << std::endl;
  *stream << "                 3 (|H(z)|^2)" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       mel-generalized line spectral pairs                           (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       spectrum                                                      (double)" << std::endl;  // NOLINT
  *stream << "  notice:" << std::endl;
  *stream << "       if k is 2, input length is assumed to be m instead of m+1" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a mglsp2sp [ @e option ] [ @e infile ]
 *
 * - @b -m @e int
 *   - order of line spectral pairs @f$(0 \le M)@f$
 * - @b -a @e double
 *   - all-pass constant @f$(|\alpha| < 1)@f$
 * - @b -g @e double
 *   - gamma @f$(-1.0 \le \gamma < 0)@f$
 * - @b -c @e int
 *   - gamma @f$\gamma = -1 / C@f$ @f$(1 \le C)@f$
 * - @b -l @e int
 *   - FFT length @f$(1 \le L)@f$
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
 *     \arg @c 1 frequency in @f$\pi@f$ rad
 *     \arg @c 2 frequency in kHz
 *     \arg @c 3 frequency in Hz
 * - @b -o @e int
 *   - output format
 *     \arg @c 0 @f$20 \log_{10}|H(z)|@f$
 *     \arg @c 1 @f$\log|H(z)|@f$
 *     \arg @c 2 @f$|H(z)|@f$
 *     \arg @c 3 @f$|H(z)|^2@f$
 * - @b infile @e str
 *   - double-type mel-LSP
 * - @b stdout
 *   - double-type spectrum
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);
  double alpha(kDefaultAlpha);
  double gamma(kDefaultGamma);
  int fft_length(kDefaultFftLength);
  double sampling_rate(kDefaultSamplingRate);
  InputGainType input_gain_type(kDefaultInputGainType);
  InputFormats input_format(kDefaultInputFormat);
  OutputFormats output_format(kDefaultOutputFormat);

  for (;;) {
    const int option_char(
        getopt_long(argc, argv, "m:a:g:c:l:s:k:q:o:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("mglsp2sp", error_message);
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
          sptk::PrintErrorMessage("mglsp2sp", error_message);
          return 1;
        }
        break;
      }
      case 'g': {
        if (!sptk::ConvertStringToDouble(optarg, &gamma) || gamma < -1.0 ||
            0.0 <= gamma) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -g option must be in [-1.0, 0.0)";
          sptk::PrintErrorMessage("mglsp2sp", error_message);
          return 1;
        }
        break;
      }
      case 'c': {
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) || tmp < 1) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -c option must be a positive integer";
          sptk::PrintErrorMessage("mglsp2sp", error_message);
          return 1;
        }
        gamma = -1.0 / tmp;
        break;
      }
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &fft_length) ||
            fft_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("mglsp2sp", error_message);
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
          sptk::PrintErrorMessage("mglsp2sp", error_message);
          return 1;
        }
        break;
      }
      case 'k': {
        const int min(0);
        const int max(static_cast<int>(kNumInputGainTypes) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -k option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("mglsp2sp", error_message);
          return 1;
        }
        input_gain_type = static_cast<InputGainType>(tmp);
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
          sptk::PrintErrorMessage("mglsp2sp", error_message);
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
          sptk::PrintErrorMessage("mglsp2sp", error_message);
          return 1;
        }
        output_format = static_cast<OutputFormats>(tmp);
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
    sptk::PrintErrorMessage("mglsp2sp", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("mglsp2sp", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::MelGeneralizedLineSpectralPairsToSpectrum
      mel_generalized_line_spectral_pairs_to_spectrum(num_order, alpha, gamma,
                                                      fft_length);
  if (!mel_generalized_line_spectral_pairs_to_spectrum.IsValid()) {
    std::ostringstream error_message;
    error_message
        << "Failed to initialize MelGeneralizedLineSpectralPairsToSpectrum";
    sptk::PrintErrorMessage("mglsp2sp", error_message);
    return 1;
  }

  const int input_length(num_order + 1);
  const int output_length(fft_length / 2 + 1);
  const int read_size(kWithoutGain == input_gain_type ? num_order
                                                      : input_length);
  const int read_point(kWithoutGain == input_gain_type ? 1 : 0);
  std::vector<double> mel_generalized_line_spectral_pairs(input_length);
  std::vector<double> spectrum(output_length);

  while (sptk::ReadStream(false, 0, read_point, read_size,
                          &mel_generalized_line_spectral_pairs, &input_stream,
                          NULL)) {
    switch (input_gain_type) {
      case kLinearGain: {
        // nothing to do
        break;
      }
      case kLogGain: {
        mel_generalized_line_spectral_pairs[0] =
            std::exp(mel_generalized_line_spectral_pairs[0]);
        break;
      }
      case kWithoutGain: {
        mel_generalized_line_spectral_pairs[0] = 1.0;
        break;
      }
      default: {
        break;
      }
    }

    switch (input_format) {
      case kFrequencyInRadians: {
        // nothing to do
        break;
      }
      case kFrequencyInCycles: {
        std::transform(mel_generalized_line_spectral_pairs.begin() + 1,
                       mel_generalized_line_spectral_pairs.end(),
                       mel_generalized_line_spectral_pairs.begin() + 1,
                       [](double x) { return x * sptk::kTwoPi; });
        break;
      }
      case kFrequencyInkHz: {
        std::transform(mel_generalized_line_spectral_pairs.begin() + 1,
                       mel_generalized_line_spectral_pairs.end(),
                       mel_generalized_line_spectral_pairs.begin() + 1,
                       [sampling_rate](double x) {
                         return x * sptk::kTwoPi / sampling_rate;
                       });
        break;
      }
      case kFrequencyInHz: {
        std::transform(mel_generalized_line_spectral_pairs.begin() + 1,
                       mel_generalized_line_spectral_pairs.end(),
                       mel_generalized_line_spectral_pairs.begin() + 1,
                       [sampling_rate](double x) {
                         return x * sptk::kTwoPi * 0.001 / sampling_rate;
                       });
        break;
      }
      default: {
        break;
      }
    }

    if (!mel_generalized_line_spectral_pairs_to_spectrum.Run(
            mel_generalized_line_spectral_pairs, &spectrum)) {
      std::ostringstream error_message;
      error_message << "Failed to line spectral pairs to spectrum";
      sptk::PrintErrorMessage("mglsp2sp", error_message);
      return 1;
    }

    switch (output_format) {
      case kLogAmplitudeSpectrumInDecibels: {
        std::transform(spectrum.begin(), spectrum.end(), spectrum.begin(),
                       [](double x) { return x * sptk::kNeper; });
        break;
      }
      case kLogAmplitudeSpectrum: {
        // nothing to do
        break;
      }
      case kAmplitudeSpectrum: {
        std::transform(spectrum.begin(), spectrum.end(), spectrum.begin(),
                       [](double x) { return std::exp(x); });
        break;
      }
      case kPowerSpectrum: {
        std::transform(spectrum.begin(), spectrum.end(), spectrum.begin(),
                       [](double x) { return std::exp(2.0 * x); });
        break;
      }
      default: {
        break;
      }
    }

    if (!sptk::WriteStream(0, output_length, spectrum, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write spectrum";
      sptk::PrintErrorMessage("mglsp2sp", error_message);
      return 1;
    }
  }

  return 0;
}
