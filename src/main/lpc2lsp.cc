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
//                1996-2018  Nagoya Institute of Technology          //
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

#include <getopt.h>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "SPTK/converter/linear_predictive_coefficients_to_line_spectral_pairs.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum OutputGainType {
  kLinearGain = 0,
  kLogGain,
  kWithoutGain,
  kNumOutputGainTypes
};

enum OutputFormats {
  kNormalizedFrequencyInRadians = 0,
  kNormalizedFrequencyInCycles,
  kFrequecnyInkHz,
  kFrequecnyInHz,
  kNumOutputFormats
};

const int kDefaultNumOrder(25);
const double kDefaultSamplingFrequency(10.0);
const OutputGainType kDefaultOutputGainType(kLinearGain);
const OutputFormats kDefaultOutputFormat(kNormalizedFrequencyInRadians);
const int kDefaultNumSplit(256);
const int kDefaultNumIteration(4);
const double kDefaultConvergenceThreshold(1e-6);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " lpc2lsp - transform linear predictive coefficients to line spectral pairs" << std::endl;  // NOLINT
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
  *stream << "                 0 (normalized frequency [0...pi])" << std::endl;
  *stream << "                 1 (normalized frequency [0...1/2])" << std::endl;
  *stream << "                 2 (frequency [kHz])" << std::endl;
  *stream << "                 3 (frequency [Hz])" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "     (level 2)" << std::endl;
  *stream << "       -n n  : number of splits of unit circle         (   int)[" << std::setw(5) << std::right << kDefaultNumSplit             << "][   0 <  n <=   ]" << std::endl;  // NOLINT
  *stream << "       -i i  : maximum number of iterations            (   int)[" << std::setw(5) << std::right << kDefaultNumIteration         << "][   0 <  i <=   ]" << std::endl;  // NOLINT
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

  // get input file
  const int num_rest_args(argc - optind);
  if (1 < num_rest_args) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("lpc2lsp", error_message);
    return 1;
  }
  const char* input_file(0 == num_rest_args ? NULL : argv[optind]);

  // open stream
  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("lpc2lsp", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  // prepare to transform
  sptk::LinearPredictiveCoefficientsToLineSpectralPairs
      linear_predictive_coefficients_to_line_spectral_pairs(
          num_order, num_split, num_iteration, convergence_threshold);
  sptk::LinearPredictiveCoefficientsToLineSpectralPairs::Buffer buffer;
  if (!linear_predictive_coefficients_to_line_spectral_pairs.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to set condition for transformation";
    sptk::PrintErrorMessage("lpc2lsp", error_message);
    return 1;
  }

  const int length(num_order + 1);
  const int write_size(kWithoutGain == output_gain_type ? num_order : length);
  const int begin(kWithoutGain == output_gain_type ? 1 : 0);
  std::vector<double> linear_predictive_coefficients(length);
  std::vector<double> line_spectral_pairs(length);

  while (sptk::ReadStream(false, 0, 0, length, &linear_predictive_coefficients,
                          &input_stream, NULL)) {
    if (!linear_predictive_coefficients_to_line_spectral_pairs.Run(
            linear_predictive_coefficients, &line_spectral_pairs, &buffer)) {
      std::ostringstream error_message;
      error_message << "Failed to transform linear predictive coefficients to "
                       "line spectral pairs";
      sptk::PrintErrorMessage("lpc2lsp", error_message);
      return 1;
    }

    switch (output_format) {
      case kNormalizedFrequencyInRadians: {
        std::transform(line_spectral_pairs.begin() + 1,
                       line_spectral_pairs.end(),
                       line_spectral_pairs.begin() + 1,
                       std::bind1st(std::multiplies<double>(), sptk::kTwoPi));
        break;
      }
      case kNormalizedFrequencyInCycles: {
        // nothing to do
        break;
      }
      case kFrequecnyInkHz: {
        std::transform(
            line_spectral_pairs.begin() + 1, line_spectral_pairs.end(),
            line_spectral_pairs.begin() + 1,
            std::bind1st(std::multiplies<double>(), sampling_frequency));
        break;
      }
      case kFrequecnyInHz: {
        std::transform(line_spectral_pairs.begin() + 1,
                       line_spectral_pairs.end(),
                       line_spectral_pairs.begin() + 1,
                       std::bind1st(std::multiplies<double>(),
                                    1000.0 * sampling_frequency));
        break;
      }
      default: { break; }
    }

    switch (output_gain_type) {
      case kLinearGain: {
        // nothing to do
        break;
      }
      case kLogGain: {
        line_spectral_pairs[0] = std::log(line_spectral_pairs[0]);
        break;
      }
      case kWithoutGain: {
        // nothing to do
        break;
      }
      default: { break; }
    }

    if (!sptk::WriteStream(begin, write_size, line_spectral_pairs, &std::cout,
                           NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write line spectral pairs";
      sptk::PrintErrorMessage("lpc2lsp", error_message);
      return 1;
    }
  }

  return 0;
}
