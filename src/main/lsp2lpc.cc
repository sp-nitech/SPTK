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
#include "SPTK/conversion/line_spectral_pairs_to_linear_predictive_coefficients.h"
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

const int kDefaultNumOrder(25);
const double kDefaultSamplingFrequency(10.0);
const InputGainType kDefaultInputGainType(kLinearGain);
const InputFormats kDefaultInputFormat(kFrequencyInRadians);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " lsp2lpc - convert line spectral pairs to linear predictive coefficients" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       lsp2lpc [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -m m  : order of line spectral pairs (   int)[" << std::setw(5) << std::right << kDefaultNumOrder          << "][   0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -s s  : sampling frequency           (double)[" << std::setw(5) << std::right << kDefaultSamplingFrequency << "][ 0.0 <  s <=   ]" << std::endl;  // NOLINT
  *stream << "       -k k  : input gain type              (   int)[" << std::setw(5) << std::right << kDefaultInputGainType     << "][   0 <= k <= 2 ]" << std::endl;  // NOLINT
  *stream << "                 0 (linear gain)" << std::endl;
  *stream << "                 1 (log gain)" << std::endl;
  *stream << "                 2 (without gain)" << std::endl;
  *stream << "       -q q  : input format                 (   int)[" << std::setw(5) << std::right << kDefaultInputFormat       << "][   0 <= q <= 3 ]" << std::endl;  // NOLINT
  *stream << "                 0 (frequency [rad])" << std::endl;
  *stream << "                 1 (frequency [cyc])" << std::endl;
  *stream << "                 2 (frequency [kHz])" << std::endl;
  *stream << "                 3 (frequency [Hz])" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       line spectral pairs                  (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       linear predictive coefficients       (double)" << std::endl;  // NOLINT
  *stream << "  notice:" << std::endl;
  *stream << "       if k is 2, input length in a frame is assumed to be m instead of m+1" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a lsp2lpc [ @e option ] [ @e infile ]
 *
 * - @b -m @e int
 *   - order of coefficients @f$(0 \le M)@f$
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
 *     \arg @c 1 frequency in cyc
 *     \arg @c 2 frequency in kHz
 *     \arg @c 3 frequency in Hz
 * - @b infile @e str
 *   - double-type LSP coefficients
 * - @b stdout
 *   - double-type LPC coefficients
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);
  double sampling_frequency(kDefaultSamplingFrequency);
  InputGainType input_gain_type(kDefaultInputGainType);
  InputFormats input_format(kDefaultInputFormat);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "m:s:k:q:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("lsp2lpc", error_message);
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
          sptk::PrintErrorMessage("lsp2lpc", error_message);
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
          sptk::PrintErrorMessage("lsp2lpc", error_message);
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
          sptk::PrintErrorMessage("lsp2lpc", error_message);
          return 1;
        }
        input_format = static_cast<InputFormats>(tmp);
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
    sptk::PrintErrorMessage("lsp2lpc", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("lsp2lpc", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::LineSpectralPairsToLinearPredictiveCoefficients
      line_spectral_pairs_to_linear_predictive_coefficients(num_order);
  sptk::LineSpectralPairsToLinearPredictiveCoefficients::Buffer buffer;
  if (!line_spectral_pairs_to_linear_predictive_coefficients.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize "
                     "LineSpectralPairsToLinearPredictiveCoefficients";
    sptk::PrintErrorMessage("lsp2lpc", error_message);
    return 1;
  }

  const int length(num_order + 1);
  const int read_size(kWithoutGain == input_gain_type ? num_order : length);
  const int read_point(kWithoutGain == input_gain_type ? 1 : 0);
  std::vector<double> coefficients(length);

  while (sptk::ReadStream(false, 0, read_point, read_size, &coefficients,
                          &input_stream, NULL)) {
    switch (input_gain_type) {
      case kLinearGain: {
        // nothing to do
        break;
      }
      case kLogGain: {
        coefficients[0] = std::exp(coefficients[0]);
        break;
      }
      case kWithoutGain: {
        coefficients[0] = 1.0;
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
        std::transform(coefficients.begin() + 1, coefficients.end(),
                       coefficients.begin() + 1,
                       [](double w) { return w * sptk::kTwoPi; });
        break;
      }
      case kFrequencyInkHz: {
        std::transform(coefficients.begin() + 1, coefficients.end(),
                       coefficients.begin() + 1,
                       [sampling_frequency](double w) {
                         return w * sptk::kTwoPi / sampling_frequency;
                       });
        break;
      }
      case kFrequencyInHz: {
        std::transform(coefficients.begin() + 1, coefficients.end(),
                       coefficients.begin() + 1,
                       [sampling_frequency](double w) {
                         return w * sptk::kTwoPi * 0.001 / sampling_frequency;
                       });
        break;
      }
      default: {
        break;
      }
    }

    if (!line_spectral_pairs_to_linear_predictive_coefficients.Run(
            &coefficients, &buffer)) {
      std::ostringstream error_message;
      error_message << "Failed to transform line spectral pairs to "
                    << "linear predictive coefficients";
      sptk::PrintErrorMessage("lsp2lpc", error_message);
      return 1;
    }

    if (!sptk::WriteStream(0, length, coefficients, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write linear predictive coefficients";
      sptk::PrintErrorMessage("lsp2lpc", error_message);
      return 1;
    }
  }

  return 0;
}
