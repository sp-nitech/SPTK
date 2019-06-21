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

#include <getopt.h>    // getopt_long
#include <algorithm>   // std::transform
#include <cmath>       // std::exp
#include <fstream>     // std::ifstream
#include <functional>  // std::bind1st, std::multiplies, std::ptr_fun
#include <iomanip>     // std::setw
#include <iostream>    // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>     // std::ostringstream
#include <vector>      // std::vector

#include "SPTK/converter/mel_generalized_line_spectral_pairs_to_spectrum.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum InputGainType {
  kLinearGain = 0,
  kLogGain,
  kWithoutGain,
  kNumInputGainTypes
};

enum InputFormats {
  kNormalizedFrequencyInRadians = 0,
  kNormalizedFrequencyInCycles,
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
const int kDefaultSpectrumLength(128);
const double kDefaultSamplingFrequency(10.0);
const InputGainType kDefaultInputGainType(kLinearGain);
const InputFormats kDefaultInputFormat(kNormalizedFrequencyInRadians);
const OutputFormats kDefaultOutputFormat(kLogAmplitudeSpectrumInDecibels);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " mglsp2sp - transform mel-generalized line spectral pairs to spectrum" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       mglsp2sp [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -m m  : order of mel-generalized line spectral pairs          (   int)[" << std::setw(5) << std::right << kDefaultNumOrder          << "][    0 <= m <=     ]" << std::endl;  // NOLINT
  *stream << "       -a a  : alpha of mel-generalized line spectral pairs          (double)[" << std::setw(5) << std::right << kDefaultAlpha             << "][ -1.0 <  a <  1.0 ]" << std::endl;  // NOLINT
  *stream << "       -g g  : gamma of mel-generalized line spectral pairs          (double)[" << std::setw(5) << std::right << kDefaultGamma             << "][ -1.0 <= g <  0.0 ]" << std::endl;  // NOLINT
  *stream << "       -c c  : gamma of mel-generalized line spectral pairs = -1 / c (   int)[" << std::setw(5) << std::right << "N/A"                     << "][    1 <= c <=     ]" << std::endl;  // NOLINT
  *stream << "       -l l  : spectrum legnth                                       (   int)[" << std::setw(5) << std::right << kDefaultSpectrumLength    << "][    0 <  l <=     ]" << std::endl;  // NOLINT
  *stream << "       -s s  : sampling frequency                                    (double)[" << std::setw(5) << std::right << kDefaultSamplingFrequency << "][  0.0 <  s <=     ]" << std::endl;  // NOLINT
  *stream << "       -k k  : input gain type                                       (   int)[" << std::setw(5) << std::right << kDefaultInputGainType     << "][    0 <= k <= 2   ]" << std::endl;  // NOLINT
  *stream << "                 0 (linear gain)" << std::endl;
  *stream << "                 1 (log gain)" << std::endl;
  *stream << "                 2 (without gain)" << std::endl;
  *stream << "       -q q  : input format                                          (   int)[" << std::setw(5) << std::right << kDefaultInputFormat       << "][    0 <= q <= 3   ]" << std::endl;  // NOLINT
  *stream << "                 0 (normalized frequency [0...pi])" << std::endl;
  *stream << "                 1 (normalized frequency [0...1/2])" << std::endl;
  *stream << "                 2 (frequency [kHz])" << std::endl;
  *stream << "                 3 (frequency [Hz])" << std::endl;
  *stream << "       -o o  : output format                                         (   int)[" << std::setw(5) << std::right << kDefaultOutputFormat      << "][    0 <= o <= 3   ]" << std::endl;  // NOLINT
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
  *stream << "       if k is 2, input length in a frame is assumed to be m instead of m+1" << std::endl;  // NOLINT
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
  int spectrum_length(kDefaultSpectrumLength);
  double sampling_frequency(kDefaultSamplingFrequency);
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
        if (!sptk::ConvertStringToInteger(optarg, &spectrum_length) ||
            spectrum_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("mglsp2sp", error_message);
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

  // get input file
  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("mglsp2sp", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  // open stream
  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("mglsp2sp", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  // prepare to transform
  sptk::MelGeneralizedLineSpectralPairsToSpectrum
      mel_generalized_line_spectral_pairs_to_spectrum(num_order, alpha, gamma,
                                                      spectrum_length);
  if (!mel_generalized_line_spectral_pairs_to_spectrum.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to set condition for transformation";
    sptk::PrintErrorMessage("mglsp2sp", error_message);
    return 1;
  }

  const int input_length(num_order + 1);
  const int output_length(spectrum_length + 1);
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
      default: { break; }
    }

    switch (input_format) {
      case kNormalizedFrequencyInRadians: {
        // nothing to do
        break;
      }
      case kNormalizedFrequencyInCycles: {
        std::transform(mel_generalized_line_spectral_pairs.begin() + 1,
                       mel_generalized_line_spectral_pairs.end(),
                       mel_generalized_line_spectral_pairs.begin() + 1,
                       std::bind1st(std::multiplies<double>(), sptk::kTwoPi));
        break;
      }
      case kFrequencyInkHz: {
        std::transform(mel_generalized_line_spectral_pairs.begin() + 1,
                       mel_generalized_line_spectral_pairs.end(),
                       mel_generalized_line_spectral_pairs.begin() + 1,
                       std::bind1st(std::multiplies<double>(),
                                    sptk::kTwoPi / sampling_frequency));
        break;
      }
      case kFrequencyInHz: {
        std::transform(mel_generalized_line_spectral_pairs.begin() + 1,
                       mel_generalized_line_spectral_pairs.end(),
                       mel_generalized_line_spectral_pairs.begin() + 1,
                       std::bind1st(std::multiplies<double>(),
                                    sptk::kTwoPi * 0.001 / sampling_frequency));
        break;
      }
      default: { break; }
    }

    if (!mel_generalized_line_spectral_pairs_to_spectrum.Run(
            mel_generalized_line_spectral_pairs, &spectrum)) {
      std::ostringstream error_message;
      error_message << "Failed to transform mel-generalized line spectral "
                    << "pairs to spectrum";
      sptk::PrintErrorMessage("mglsp2sp", error_message);
      return 1;
    }

    switch (output_format) {
      case kLogAmplitudeSpectrumInDecibels: {
        std::transform(spectrum.begin(), spectrum.end(), spectrum.begin(),
                       std::bind1st(std::multiplies<double>(), sptk::kNeper));
        break;
      }
      case kLogAmplitudeSpectrum: {
        // nothing to do
        break;
      }
      case kAmplitudeSpectrum: {
        std::transform(spectrum.begin(), spectrum.end(), spectrum.begin(),
                       std::ptr_fun<double, double>(std::exp));
        break;
      }
      case kPowerSpectrum: {
        std::transform(spectrum.begin(), spectrum.end(), spectrum.begin(),
                       std::ptr_fun<double, double>(
                           [](double x) { return std::exp(2.0 * x); }));
        break;
      }
      default: { break; }
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
