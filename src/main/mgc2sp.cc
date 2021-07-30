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
//                1996-2021  Nagoya Institute of Technology          //
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

#include <getopt.h>  // getopt_long

#include <algorithm>  // std::transform
#include <cmath>      // std::exp
#include <fstream>    // std::ifstream
#include <iomanip>    // std::setw
#include <iostream>   // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>    // std::ostringstream
#include <vector>     // std::vector

#include "SPTK/conversion/mel_generalized_cepstrum_to_spectrum.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum OutputFormats {
  kLogAmplitudeSpectrumInDecibels = 0,
  kLogAmplitudeSpectrum,
  kAmplitudeSpectrum,
  kPowerSpectrum,
  kPhaseSpectrumInCycles,
  kPhaseSpectrumInRadians,
  kPhaseSpectrumInDegrees,
  kNumOutputFormats
};

const int kDefaultNumOrder(25);
const double kDefaultAlpha(0.0);
const double kDefaultGamma(0.0);
const bool kDefaultNormalizationFlag(false);
const bool kDefaultMultiplicationFlag(false);
const int kDefaultFftLength(256);
const OutputFormats kDefaultOutputFormat(kLogAmplitudeSpectrumInDecibels);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " mgc2sp - transform mel-generalized cepstral to spectrum" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       mgc2sp [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -m m  : order of mel-generalized cepstrum          (   int)[" << std::setw(5) << std::right << kDefaultNumOrder     << "][    0 <= m <=     ]" << std::endl;  // NOLINT
  *stream << "       -a a  : alpha of mel-generalized cepstrum          (double)[" << std::setw(5) << std::right << kDefaultAlpha        << "][ -1.0 <  a <  1.0 ]" << std::endl;  // NOLINT
  *stream << "       -g g  : gamma of mel-generalized cepstrum          (double)[" << std::setw(5) << std::right << kDefaultGamma        << "][ -1.0 <= g <= 1.0 ]" << std::endl;  // NOLINT
  *stream << "       -c c  : gamma of mel-generalized cepstrum = -1 / c (   int)[" << std::setw(5) << std::right << "N/A"                << "][    1 <= c <=     ]" << std::endl;  // NOLINT
  *stream << "       -n    : regard input as normalized                 (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultNormalizationFlag)  << "]" << std::endl;  // NOLINT
  *stream << "               mel-generalized cepstrum" << std::endl;
  *stream << "       -u    : regard input as multiplied by gamma        (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultMultiplicationFlag) << "]" << std::endl;  // NOLINT
  *stream << "       -l l  : FFT length                                 (   int)[" << std::setw(5) << std::right << kDefaultFftLength    << "][    2 <= l <=     ]" << std::endl;  // NOLINT
  *stream << "       -o o  : output format                              (   int)[" << std::setw(5) << std::right << kDefaultOutputFormat << "][    0 <= o <= 6   ]" << std::endl;  // NOLINT
  *stream << "                 0 (20*log|H(z)|)" << std::endl;
  *stream << "                 1 (ln|H(z)|)" << std::endl;
  *stream << "                 2 (|H(z)|)" << std::endl;
  *stream << "                 3 (|H(z)|^2)" << std::endl;
  *stream << "                 4 (arg|H(z)|/pi)" << std::endl;
  *stream << "                 5 (arg|H(z)|)" << std::endl;
  *stream << "                 6 (arg|H(z)|*180/pi)" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       mel-generalized cepstrum                           (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       spectrum                                           (double)" << std::endl;  // NOLINT
  *stream << "  notice:" << std::endl;
  *stream << "       value of l must be a power of 2" << std::endl;
  *stream << "       if -u is used without -n, input is regarded as 1+g/mgc[0],g*mgc[1],...,g*mgc[m]" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a mgc2sp [ @e option ] [ @e infile ]
 *
 * - @b -m @e int
 *   - order of coefficients @f$(0 \le M)@f$
 * - @b -a @e double
 *   - all-pass constant @f$(|\alpha| < 1)@f$
 * - @b -g @e double
 *   - gamma @f$(|\gamma| \le 1)@f$
 * - @b -c @e int
 *   - gamma @f$\gamma = -1 / C@f$ @f$(1 \le C)@f$
 * - @b -n @e bool
 *   - regard as normalized mel-generalized cepstrum
 * - @b -u @e bool
 *   - regard as multiplied by gamma
 * - @b -l @e int
 *   - FFT length @f$(2 \le N)@f$
 * - @b -o @e int
 *   - output format
 *     \arg @c 0 @f$20 \log_{10} |H(z)|@f$
 *     \arg @c 1 @f$\log |H(z)|@f$
 *     \arg @c 2 @f$|H(z)|@f$
 *     \arg @c 3 @f$|H(z)|^2@f$
 *     \arg @c 4 @f$\arg|H(z)| / \pi@f$
 *     \arg @c 5 @f$\arg|H(z)|@f$
 *     \arg @c 6 @f$\arg|H(z)| \times 180/\pi@f$
 * - @b infile @e str
 *   - double-type mel-generalized cepstral coefficients
 * - @b stdout
 *   - double-type spectrum
 *
 * In the following example, 12-th order mel-generalized cepstral coefficients
 * in @c data.mgcep are converted to log magnitude spectrum.
 *
 * @code{.sh}
 *   mgc2sp -m 12 -a 0.35 -c 2 < data.mgcep > data.spec
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);
  double alpha(kDefaultAlpha);
  double gamma(kDefaultGamma);
  bool normalization_flag(kDefaultNormalizationFlag);
  bool multiplication_flag(kDefaultMultiplicationFlag);
  int fft_length(kDefaultFftLength);
  OutputFormats output_format(kDefaultOutputFormat);

  for (;;) {
    const int option_char(
        getopt_long(argc, argv, "m:a:g:c:nul:o:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("mgc2sp", error_message);
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
          sptk::PrintErrorMessage("mgc2sp", error_message);
          return 1;
        }
        break;
      }
      case 'g': {
        if (!sptk::ConvertStringToDouble(optarg, &gamma) ||
            !sptk::IsValidGamma(gamma)) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -g option must be in [-1.0, 1.0]";
          sptk::PrintErrorMessage("mgc2sp", error_message);
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
          sptk::PrintErrorMessage("mgc2sp", error_message);
          return 1;
        }
        gamma = -1.0 / tmp;
        break;
      }
      case 'n': {
        normalization_flag = true;
        break;
      }
      case 'u': {
        multiplication_flag = true;
        break;
      }
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &fft_length)) {
          std::ostringstream error_message;
          error_message << "The argument for the -l option must be an integer";
          sptk::PrintErrorMessage("mgc2sp", error_message);
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
          sptk::PrintErrorMessage("mgc2sp", error_message);
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

  if (0.0 == gamma && multiplication_flag) {
    std::ostringstream error_message;
    error_message << "If -u option is given, gamma must not be 0";
    sptk::PrintErrorMessage("mgc2sp", error_message);
    return 1;
  }

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("mgc2sp", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("mgc2sp", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::MelGeneralizedCepstrumToSpectrum mel_generalized_cepstrum_to_spectrum(
      num_order, alpha, gamma, normalization_flag, multiplication_flag,
      fft_length);
  sptk::MelGeneralizedCepstrumToSpectrum::Buffer buffer;
  if (!mel_generalized_cepstrum_to_spectrum.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize MelGeneralizedCepstrumToSpectrum";
    sptk::PrintErrorMessage("mgc2sp", error_message);
    return 1;
  }

  const int input_length(num_order + 1);
  const int output_length(fft_length / 2 + 1);
  std::vector<double> mel_generalized_cepstrum(input_length);
  std::vector<double> amplitude_spectrum(fft_length);
  std::vector<double> phase_spectrum(fft_length);

  while (sptk::ReadStream(false, 0, 0, input_length, &mel_generalized_cepstrum,
                          &input_stream, NULL)) {
    // Perform input modification.
    if (!normalization_flag && multiplication_flag) {
      (*mel_generalized_cepstrum.begin()) =
          (*(mel_generalized_cepstrum.begin()) - 1.0) / gamma;
    }

    // Transform.
    if (!mel_generalized_cepstrum_to_spectrum.Run(mel_generalized_cepstrum,
                                                  &amplitude_spectrum,
                                                  &phase_spectrum, &buffer)) {
      std::ostringstream error_message;
      error_message
          << "Failed to transform mel-generalized ceptrum to spectrum";
      sptk::PrintErrorMessage("mgc2sp", error_message);
      return 1;
    }

    switch (output_format) {
      case kLogAmplitudeSpectrumInDecibels: {
        std::transform(amplitude_spectrum.begin(),
                       amplitude_spectrum.begin() + output_length,
                       amplitude_spectrum.begin(),
                       [](double x) { return x * sptk::kNeper; });
        break;
      }
      case kLogAmplitudeSpectrum: {
        // nothing to do
        break;
      }
      case kAmplitudeSpectrum: {
        std::transform(amplitude_spectrum.begin(),
                       amplitude_spectrum.begin() + output_length,
                       amplitude_spectrum.begin(),
                       [](double x) { return std::exp(x); });
        break;
      }
      case kPowerSpectrum: {
        std::transform(amplitude_spectrum.begin(),
                       amplitude_spectrum.begin() + output_length,
                       amplitude_spectrum.begin(),
                       [](double x) { return std::exp(2.0 * x); });
        break;
      }
      case kPhaseSpectrumInCycles: {
        std::transform(
            phase_spectrum.begin(), phase_spectrum.begin() + output_length,
            phase_spectrum.begin(), [](double x) { return x / sptk::kPi; });
        break;
      }
      case kPhaseSpectrumInRadians: {
        // nothing to do
        break;
      }
      case kPhaseSpectrumInDegrees: {
        std::transform(phase_spectrum.begin(),
                       phase_spectrum.begin() + output_length,
                       phase_spectrum.begin(),
                       [](double x) { return x * 180.0 / sptk::kPi; });
        break;
      }
      default: { break; }
    }

    switch (output_format) {
      case kLogAmplitudeSpectrumInDecibels:
      case kLogAmplitudeSpectrum:
      case kAmplitudeSpectrum:
      case kPowerSpectrum: {
        if (!sptk::WriteStream(0, output_length, amplitude_spectrum, &std::cout,
                               NULL)) {
          std::ostringstream error_message;
          error_message << "Failed to write amplitude spectrum";
          sptk::PrintErrorMessage("mgc2sp", error_message);
          return 1;
        }
        break;
      }
      case kPhaseSpectrumInCycles:
      case kPhaseSpectrumInRadians:
      case kPhaseSpectrumInDegrees: {
        if (!sptk::WriteStream(0, output_length, phase_spectrum, &std::cout,
                               NULL)) {
          std::ostringstream error_message;
          error_message << "Failed to write phase spectrum";
          sptk::PrintErrorMessage("mgc2sp", error_message);
          return 1;
        }
        break;
      }
      default: { break; }
    }
  }

  return 0;
}
