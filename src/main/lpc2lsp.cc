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
#include <cmath>      // std::log
#include <fstream>    // std::ifstream
#include <iomanip>    // std::setw
#include <iostream>   // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>    // std::ostringstream
#include <vector>     // std::vector

#include "Getopt/getoptwin.h"
#include "SPTK/conversion/linear_predictive_coefficients_to_line_spectral_pairs.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum OutputGainType {
  kLinearGain = 0,
  kLogGain,
  kWithoutGain,
  kNumOutputGainTypes
};

enum OutputFormats {
  kFrequencyInRadians = 0,
  kFrequencyInCycles,
  kFrequecnyInkHz,
  kFrequecnyInHz,
  kNumOutputFormats
};

const int kDefaultNumOrder(25);
const double kDefaultSamplingFrequency(10.0);
const OutputGainType kDefaultOutputGainType(kLinearGain);
const OutputFormats kDefaultOutputFormat(kFrequencyInRadians);
const int kDefaultNumSplit(256);
const int kDefaultNumIteration(4);
const double kDefaultConvergenceThreshold(1e-6);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " lpc2lsp - convert linear predictive coefficients to line spectral pairs" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       lpc2lsp [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -m m  : order of linear predictive coefficients (   int)[" << std::setw(5) << std::right << kDefaultNumOrder             << "][   0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -s s  : sampling frequency                      (double)[" << std::setw(5) << std::right << kDefaultSamplingFrequency    << "][ 0.0 <  s <=   ]" << std::endl;  // NOLINT
  *stream << "       -k k  : output gain type                        (   int)[" << std::setw(5) << std::right << kDefaultOutputGainType       << "][   0 <= k <= 2 ]" << std::endl;  // NOLINT
  *stream << "                 0 (linear gain)" << std::endl;
  *stream << "                 1 (log gain)" << std::endl;
  *stream << "                 2 (without gain)" << std::endl;
  *stream << "       -o o  : output format                           (   int)[" << std::setw(5) << std::right << kDefaultOutputFormat         << "][   0 <= o <= 3 ]" << std::endl;  // NOLINT
  *stream << "                 0 (frequency [rad])" << std::endl;
  *stream << "                 1 (frequency [cyc])" << std::endl;
  *stream << "                 2 (frequency [kHz])" << std::endl;
  *stream << "                 3 (frequency [Hz])" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "     (level 2)" << std::endl;
  *stream << "       -n n  : number of splits of unit circle         (   int)[" << std::setw(5) << std::right << kDefaultNumSplit             << "][   1 <= n <=   ]" << std::endl;  // NOLINT
  *stream << "       -i i  : maximum number of iterations            (   int)[" << std::setw(5) << std::right << kDefaultNumIteration         << "][   1 <= i <=   ]" << std::endl;  // NOLINT
  *stream << "       -d d  : convergence threshold                   (double)[" << std::setw(5) << std::right << kDefaultConvergenceThreshold << "][ 0.0 <= d <=   ]" << std::endl;  // NOLINT
  *stream << "  infile:" << std::endl;
  *stream << "       linear predictive coefficients                  (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       line spectral pairs                             (double)" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a lpc2lsp [ @e option ] [ @e infile ]
 *
 * - @b -m @e int
 *   - order of coefficients @f$(0 \le M)@f$
 * - @b -s @e double
 *   - sampling rate @f$(0 < F_s)@f$
 * - @b -k @e int
 *   - output gain type
 *     \arg @c 0 linear gain
 *     \arg @c 1 log gain
 *     \arg @c 2 without gain
 * - @b -o @e int
 *   - output format
 *     \arg @c 0 frequency in rad
 *     \arg @c 1 frequency in cyc
 *     \arg @c 2 frequency in kHz
 *     \arg @c 3 frequency in Hz
 * - @b -n @e int
 *   - number of splits of unit circle @f$(1 \le S)@f$
 * - @b -i @e int
 *   - maximum number of iterations @f$(1 \le N)@f$
 * - @b -d @e double
 *   - convergence threshold @f$(0 \le \epsilon)@f$
 * - @b infile @e str
 *   - double-type LPC coefficients
 * - @b stdout
 *   - double-type LSP coefficients
 *
 * The below example extracts 10-th order LSP coefficients from @c data.d.
 *
 * @code{.sh}
 *   frame < data.d | window | lpc -m 10 | lpc2lsp -m 10 > data.lsp
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);
  double sampling_frequency(kDefaultSamplingFrequency);
  OutputGainType output_gain_type(kDefaultOutputGainType);
  OutputFormats output_format(kDefaultOutputFormat);
  int num_split(kDefaultNumSplit);
  int num_iteration(kDefaultNumIteration);
  double convergence_threshold(kDefaultConvergenceThreshold);

  for (;;) {
    const int option_char(
        getopt_long(argc, argv, "m:s:k:o:n:i:d:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("lpc2lsp", error_message);
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
          sptk::PrintErrorMessage("lpc2lsp", error_message);
          return 1;
        }
        break;
      }
      case 'k': {
        const int min(0);
        const int max(static_cast<int>(kNumOutputGainTypes) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -k option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("lpc2lsp", error_message);
          return 1;
        }
        output_gain_type = static_cast<OutputGainType>(tmp);
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
          sptk::PrintErrorMessage("lpc2lsp", error_message);
          return 1;
        }
        output_format = static_cast<OutputFormats>(tmp);
        break;
      }
      case 'n': {
        if (!sptk::ConvertStringToInteger(optarg, &num_split) ||
            num_split <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -n option must be a positive integer";
          sptk::PrintErrorMessage("lpc2lsp", error_message);
          return 1;
        }
        break;
      }
      case 'i': {
        if (!sptk::ConvertStringToInteger(optarg, &num_iteration) ||
            num_iteration <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -i option must be a positive integer";
          sptk::PrintErrorMessage("lpc2lsp", error_message);
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
          sptk::PrintErrorMessage("lpc2lsp", error_message);
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
    sptk::PrintErrorMessage("lpc2lsp", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("lpc2lsp", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::LinearPredictiveCoefficientsToLineSpectralPairs
      linear_predictive_coefficients_to_line_spectral_pairs(
          num_order, num_split, num_iteration, convergence_threshold);
  sptk::LinearPredictiveCoefficientsToLineSpectralPairs::Buffer buffer;
  if (!linear_predictive_coefficients_to_line_spectral_pairs.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize "
                     "LinearPredictiveCoefficientsToLineSpectralPairs";
    sptk::PrintErrorMessage("lpc2lsp", error_message);
    return 1;
  }

  const int length(num_order + 1);
  const int write_size(kWithoutGain == output_gain_type ? num_order : length);
  const int begin(kWithoutGain == output_gain_type ? 1 : 0);
  std::vector<double> coefficients(length);

  while (sptk::ReadStream(false, 0, 0, length, &coefficients, &input_stream,
                          NULL)) {
    if (!linear_predictive_coefficients_to_line_spectral_pairs.Run(
            &coefficients, &buffer)) {
      std::ostringstream error_message;
      error_message << "Failed to convert linear predictive coefficients to "
                       "line spectral pairs";
      sptk::PrintErrorMessage("lpc2lsp", error_message);
      return 1;
    }

    switch (output_format) {
      case kFrequencyInRadians: {
        std::transform(coefficients.begin() + 1, coefficients.end(),
                       coefficients.begin() + 1,
                       [](double w) { return w * sptk::kTwoPi; });
        break;
      }
      case kFrequencyInCycles: {
        // nothing to do
        break;
      }
      case kFrequecnyInkHz: {
        std::transform(
            coefficients.begin() + 1, coefficients.end(),
            coefficients.begin() + 1,
            [sampling_frequency](double w) { return w * sampling_frequency; });
        break;
      }
      case kFrequecnyInHz: {
        std::transform(coefficients.begin() + 1, coefficients.end(),
                       coefficients.begin() + 1,
                       [sampling_frequency](double w) {
                         return w * 1000.0 * sampling_frequency;
                       });
        break;
      }
      default: {
        break;
      }
    }

    switch (output_gain_type) {
      case kLinearGain: {
        // nothing to do
        break;
      }
      case kLogGain: {
        coefficients[0] = std::log(coefficients[0]);
        break;
      }
      case kWithoutGain: {
        // nothing to do
        break;
      }
      default: {
        break;
      }
    }

    if (!sptk::WriteStream(begin, write_size, coefficients, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write line spectral pairs";
      sptk::PrintErrorMessage("lpc2lsp", error_message);
      return 1;
    }
  }

  return 0;
}
