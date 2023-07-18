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
#include "SPTK/analysis/spectrum_extraction.h"
#include "SPTK/conversion/spectrum_to_spectrum.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum InputFormats { kPitch = 0, kF0, kLogF0, kNumInputFormats };

const sptk::SpectrumExtraction::Algorithms kDefaultAlgorithm(
    sptk::SpectrumExtraction::Algorithms::kWorld);
const int kDefaultFftLength(1024);
const int kDefaultFrameShift(80);
const double kDefaultSamplingRate(16.0);
const InputFormats kDefaultInputFormat(kPitch);
const sptk::SpectrumToSpectrum::InputOutputFormats kDefaultOutputFormat(
    sptk::SpectrumToSpectrum::kLogAmplitudeSpectrumInDecibels);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " pitch_spec - pitch-adaptive spectral estimation" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       pitch_spec [ options ] f0file [ infile ] > stdout" << std::endl;  // NOLINT
  *stream << "  options:" << std::endl;
  *stream << "       -a a  : algorithm used for  (   int)[" << std::setw(5) << std::right << kDefaultAlgorithm    << "][   0 <= a <= 0    ]" << std::endl;  // NOLINT
  *stream << "               spectrum estimation" << std::endl;
  *stream << "                 0 (WORLD)" << std::endl;
  *stream << "       -l l  : FFT length          (   int)[" << std::setw(5) << std::right << kDefaultFftLength    << "][   4 <= l <=      ]" << std::endl;  // NOLINT
  *stream << "       -p p  : frame shift [point] (   int)[" << std::setw(5) << std::right << kDefaultFrameShift   << "][   1 <= p <=      ]" << std::endl;  // NOLINT
  *stream << "       -s s  : sampling rate [kHz] (double)[" << std::setw(5) << std::right << kDefaultSamplingRate << "][ 8.0 <= s <= 98.0 ]" << std::endl;  // NOLINT
  *stream << "       -q q  : f0 input format     (   int)[" << std::setw(5) << std::right << kDefaultInputFormat  << "][   0 <= q <= 2    ]" << std::endl;  // NOLINT
  *stream << "                 0 (Fs/F0)" << std::endl;
  *stream << "                 1 (F0)" << std::endl;
  *stream << "                 2 (log F0)" << std::endl;
  *stream << "       -o o  : output format       (   int)[" << std::setw(5) << std::right << kDefaultOutputFormat << "][   0 <= o <= 3    ]" << std::endl;  // NOLINT
  *stream << "                 0 (20*log|H(z)|)" << std::endl;
  *stream << "                 1 (ln|H(z)|)" << std::endl;
  *stream << "                 2 (|H(z)|)" << std::endl;
  *stream << "                 3 (|H(z)|^2)" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       waveform                    (double)[stdin]" << std::endl;
  *stream << "  f0file:" << std::endl;
  *stream << "       pitch                       (double)" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       spectrum                    (double)" << std::endl;
  *stream << "  notice:" << std::endl;
  *stream << "       value of l must be a power of 2" << std::endl;
  *stream << "       magic number representing unvoiced symbol is 0 (q = 0, 1) or -1e+10 (q = 2)" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a pitch_spec [ @e option ] @e f0file [ @e infile ]
 *
 * - @b -a @e int
 *   - algorithm used for spectrum extraction
 *     @arg @c 0 WORLD (CheapTrick)
 * - @b -l @e int
 *   - FFT length
 * - @b -p @e int
 *   - frame shift [point] @f$(1 \le P)@f$
 * - @b -s @e double
 *   - sampling rate [kHz] @f$(8 \le F_s \le 98)@f$
 * - @b -q @e int
 *   - f0 input format
 *     @arg @c 0 pitch @f$(F_s / F_0)@f$
 *     @arg @c 1 F0
 *     @arg @c 2 log F0
 * - @b -o @e int
 *   - output format
 *     \arg @c 0 @f$20 \log_{10} |H(z)|@f$
 *     \arg @c 1 @f$\log |H(z)|@f$
 *     \arg @c 2 @f$|H(z)|@f$
 *     \arg @c 3 @f$|H(z)|^2@f$
 * - @b infile @e str
 *   - double-type waveform
 * - @b f0file @e str
 *   - double-type pitch
 * - @b stdout
 *   - double-type spectrum
 *
 * The below is a simple example to extract spectrum from @c data.d.
 *
 * @code{.sh}
 *   pitch -s 16 -p 80 -L 80 -H 200 -o 1 < data.d > data.f0
 *   pitch_spec -s 16 -p 80 -q 1 data.f0 < data.d > data.sp
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  sptk::SpectrumExtraction::Algorithms algorithm(kDefaultAlgorithm);
  int fft_length(kDefaultFftLength);
  int frame_shift(kDefaultFrameShift);
  double sampling_rate(kDefaultSamplingRate);
  InputFormats input_format(kDefaultInputFormat);
  sptk::SpectrumToSpectrum::InputOutputFormats output_format(
      kDefaultOutputFormat);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "a:l:p:s:q:o:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'a': {
        const int min(0);
        const int max(
            static_cast<int>(
                sptk::SpectrumExtraction::Algorithms::kNumAlgorithms) -
            1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -a option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("pitch_spec", error_message);
          return 1;
        }
        algorithm = static_cast<sptk::SpectrumExtraction::Algorithms>(tmp);
        break;
      }
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &fft_length) ||
            fft_length <= 3) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be greater than 3";
          sptk::PrintErrorMessage("pitch_spec", error_message);
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
          sptk::PrintErrorMessage("pitch_spec", error_message);
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
          sptk::PrintErrorMessage("pitch_spec", error_message);
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
          sptk::PrintErrorMessage("pitch_spec", error_message);
          return 1;
        }
        input_format = static_cast<InputFormats>(tmp);
        break;
      }
      case 'o': {
        const int min(0);
        const int max(
            static_cast<int>(sptk::SpectrumToSpectrum::kNumInputOutputFormats) -
            1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -o option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("pitch_spec", error_message);
          return 1;
        }
        output_format =
            static_cast<sptk::SpectrumToSpectrum::InputOutputFormats>(tmp);
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

  const char* f0_file;
  const char* raw_file;
  const int num_input_files(argc - optind);
  if (2 == num_input_files) {
    f0_file = argv[argc - 2];
    raw_file = argv[argc - 1];
  } else if (1 == num_input_files) {
    f0_file = argv[argc - 1];
    raw_file = NULL;
  } else {
    std::ostringstream error_message;
    error_message << "Just two input files, f0file and infile, are required";
    sptk::PrintErrorMessage("pitch_spec", error_message);
    return 1;
  }

  if (!sptk::SetBinaryMode()) {
    std::ostringstream error_message;
    error_message << "Cannot set translation mode";
    sptk::PrintErrorMessage("pitch_spec", error_message);
    return 1;
  }
  const double sampling_rate_in_hz(1000.0 * sampling_rate);

  std::vector<double> f0;
  {
    std::ifstream ifs;
    ifs.open(f0_file, std::ios::in | std::ios::binary);
    if (ifs.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << f0_file;
      sptk::PrintErrorMessage("pitch_spec", error_message);
      return 1;
    }
    std::istream& input_stream(ifs);

    double tmp;
    while (sptk::ReadStream(&tmp, &input_stream)) {
      f0.push_back(tmp);
    }

    switch (input_format) {
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

  std::vector<double> waveform;
  {
    std::ifstream ifs;
    if (NULL != raw_file) {
      ifs.open(raw_file, std::ios::in | std::ios::binary);
      if (ifs.fail()) {
        std::ostringstream error_message;
        error_message << "Cannot open file " << raw_file;
        sptk::PrintErrorMessage("pitch_spec", error_message);
        return 1;
      }
    }
    std::istream& input_stream(ifs.is_open() ? ifs : std::cin);

    double tmp;
    while (sptk::ReadStream(&tmp, &input_stream)) {
      waveform.push_back(tmp);
    }
  }
  if (waveform.empty()) return 0;

  sptk::SpectrumExtraction spectrum_extraction(fft_length, frame_shift,
                                               sampling_rate_in_hz, algorithm);
  if (!spectrum_extraction.IsValid()) {
    std::ostringstream error_message;
    error_message << "FFT length must be a power of 2";
    sptk::PrintErrorMessage("pitch_spec", error_message);
    return 1;
  }

  sptk::SpectrumToSpectrum spectrum_to_spectrum(
      fft_length, sptk::SpectrumToSpectrum::kPowerSpectrum, output_format);

  std::vector<std::vector<double> > spectrum;
  if (!spectrum_extraction.Run(waveform, f0, &spectrum)) {
    std::ostringstream error_message;
    error_message << "Failed to extract spectrum";
    sptk::PrintErrorMessage("pitch_spec", error_message);
    return 1;
  }

  const int output_length(fft_length / 2 + 1);
  std::vector<double> output(output_length);

  for (const std::vector<double>& tmp : spectrum) {
    if (!spectrum_to_spectrum.Run(tmp, &output)) {
      std::ostringstream error_message;
      error_message << "Failed to convert spectrum";
      sptk::PrintErrorMessage("pitch_spec", error_message);
      return 1;
    }

    if (!sptk::WriteStream(0, output_length, output, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write spectrum";
      sptk::PrintErrorMessage("pitch_spec", error_message);
      return 1;
    }
  }

  return 0;
}
