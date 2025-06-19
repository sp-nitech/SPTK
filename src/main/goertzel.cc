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

#include <cmath>     // std::sqrt
#include <fstream>   // std::ifstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream
#include <vector>    // std::vector

#include "GETOPT/ya_getopt.h"
#include "SPTK/analysis/goertzel_analysis.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum OutputFormats {
  kOutputRealAndImagParts = 0,
  kOutputRealPart,
  kOutputImagPart,
  kOutputAmplitude,
  kOutputPower,
  kNumOutputFormats
};

const int kDefaultFftLength(256);
const double kDefaultSamplingRate(16.0);
const double kDefaultFrequency(0.0);
const OutputFormats kDefaultOutputFormat(kOutputRealAndImagParts);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " goertzel - Frequency analysis using Goertzel algorithm" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       goertzel [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l          : FFT length          (   int)[" << std::setw(5) << std::right << kDefaultFftLength    << "][ 1 <= l <=       ]" << std::endl;  // NOLINT
  *stream << "       -m m          : order of sequence   (   int)[" << std::setw(5) << std::right << "l-1"                << "][ 0 <= m <        ]" << std::endl;  // NOLINT
  *stream << "       -s s          : sampling rate [kHz] (double)[" << std::setw(5) << std::right << kDefaultSamplingRate << "][ 0 <  s <=       ]" << std::endl;  // NOLINT
  *stream << "       -f f1 f2 ...  : frequencies         (double)[" << std::setw(5) << std::right << kDefaultFrequency    << "][ 0 <= f <  500*s ]" << std::endl;  // NOLINT
  *stream << "       -o o          : output format       (   int)[" << std::setw(5) << std::right << kDefaultOutputFormat << "][ 0 <= o <= 4     ]" << std::endl;  // NOLINT
  *stream << "                         0 (real and imaginary parts)" << std::endl;  // NOLINT
  *stream << "                         1 (real part)" << std::endl;
  *stream << "                         2 (imaginary part)" << std::endl;
  *stream << "                         3 (amplitude)" << std::endl;
  *stream << "                         4 (power)" << std::endl;
  *stream << "       -h            : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence                       (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       DFT sequence                        (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a goertzel [ @e option ] [ @e infile ]
 *
 * - @b -l @e int
 *   - FFT length @f$(1 \le L)@f$
 * - @b -m @e int
 *   - order of sequence @f$(0 \le M)@f$
 * - @b -s @e double
 *   - sampling rate [kHz] @f$(0 < F_s)@f$
 * - @b -f @e double+
 *   - frequencies [Hz] @f$(0 \le F_k < 500F_s)@f$
 * - @b -o @e int
 *   - output format
 *     \arg @c 0 real and imaginary parts
 *     \arg @c 1 real part
 *     \arg @c 2 imaginary part
 *     \arg @c 3 amplitude
 *     \arg @c 4 power
 * - @b infile @e str
 *   - double-type data sequence
 * - @b stdout
 *   - double-type DFT sequence
 *
 * The below example analyzes a sine wave using Goertzel algorithm.
 *
 * @code{.sh}
 *   sin -p 30 -l 256 | goertzel -l 256 -o 3 > sine.amp
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int fft_length(kDefaultFftLength);
  int num_order(kDefaultFftLength - 1);
  bool is_num_order_specified(false);
  double sampling_rate(kDefaultSamplingRate);
  std::vector<double> frequencies(1, kDefaultFrequency);
  OutputFormats output_format(kDefaultOutputFormat);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:s:f:o:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &fft_length) ||
            fft_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("goertzel", error_message);
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
          sptk::PrintErrorMessage("goertzel", error_message);
          return 1;
        }
        is_num_order_specified = true;
        break;
      }
      case 's': {
        if (!sptk::ConvertStringToDouble(optarg, &sampling_rate) ||
            sampling_rate < 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -s option must be a non-negative number";
          sptk::PrintErrorMessage("goertzel", error_message);
          return 1;
        }
        break;
      }
      case 'f': {
        frequencies.clear();
        double frequency;
        if (!sptk::ConvertStringToDouble(optarg, &frequency) ||
            frequency < 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -f option must be a non-negative number";
          sptk::PrintErrorMessage("goertzel", error_message);
          return 1;
        }
        frequencies.push_back(frequency);
        while (optind < argc &&
               sptk::ConvertStringToDouble(argv[optind], &frequency) &&
               0.0 <= frequency) {
          frequencies.push_back(frequency);
          ++optind;
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
          sptk::PrintErrorMessage("goertzel", error_message);
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

  const double sampling_rate_in_hz(1000.0 * sampling_rate);
  for (double frequency : frequencies) {
    if (0.5 * sampling_rate_in_hz <= frequency) {
      std::ostringstream error_message;
      error_message << "Frequency must be less than Nyquist frequency";
      sptk::PrintErrorMessage("goertzel", error_message);
      return 1;
    }
  }

  if (!is_num_order_specified) {
    num_order = fft_length - 1;
  }

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("goertzel", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  if (!sptk::SetBinaryMode()) {
    std::ostringstream error_message;
    error_message << "Cannot set translation mode";
    sptk::PrintErrorMessage("goertzel", error_message);
    return 1;
  }

  std::ifstream ifs;
  if (NULL != input_file) {
    ifs.open(input_file, std::ios::in | std::ios::binary);
    if (ifs.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << input_file;
      sptk::PrintErrorMessage("goertzel", error_message);
      return 1;
    }
  }
  std::istream& input_stream(ifs.is_open() ? ifs : std::cin);

  sptk::GoertzelAnalysis goertzel_analysis(sampling_rate_in_hz, frequencies,
                                           fft_length);
  if (!goertzel_analysis.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize GoertzelAnalysis";
    sptk::PrintErrorMessage("goertzel", error_message);
    return 1;
  }

  const int input_length(num_order + 1);
  const int output_length(static_cast<int>(frequencies.size()));
  std::vector<double> input_x(input_length);
  std::vector<double> output_x(output_length);
  std::vector<double> output_y(output_length);

  while (sptk::ReadStream(true, 0, 0, input_length, &input_x, &input_stream,
                          NULL)) {
    if (!goertzel_analysis.Run(input_x, &output_x, &output_y)) {
      std::ostringstream error_message;
      error_message << "Failed to perform Goertzel analysis";
      sptk::PrintErrorMessage("goertzel", error_message);
      return 1;
    }

    if (kOutputAmplitude == output_format) {
      for (int i(0); i < output_length; ++i) {
        output_x[i] =
            std::sqrt(output_x[i] * output_x[i] + output_y[i] * output_y[i]);
      }
    } else if (kOutputPower == output_format) {
      for (int i(0); i < output_length; ++i) {
        output_x[i] = output_x[i] * output_x[i] + output_y[i] * output_y[i];
      }
    }

    if (kOutputImagPart != output_format &&
        !sptk::WriteStream(0, output_length, output_x, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write output sequence";
      sptk::PrintErrorMessage("goertzel", error_message);
      return 1;
    }

    if ((kOutputRealAndImagParts == output_format ||
         kOutputImagPart == output_format) &&
        !sptk::WriteStream(0, output_length, output_y, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write imaginary parts";
      sptk::PrintErrorMessage("goertzel", error_message);
      return 1;
    }
  }

  return 0;
}
