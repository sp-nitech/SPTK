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
//                1996-2017  Nagoya Institute of Technology          //
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
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "SPTK/analyzer/adaptive_mel_cepstral_analyzer.h"
#include "SPTK/math/statistics_accumulator.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultNumOrder(25);
const double kDefaultAlpha(0.35);
const double kDefaultMinimumEpsilon(1e-16);
const double kDefaultMomentum(0.9);
const double kDefaultForgettingFactor(0.98);
const double kDefaultStepSizeFactor(0.1);
const int kDefaultOutputPeriod(1);
const int kDefaultNumPadeOrder(4);
const bool kDefaultAverageFlag(false);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " amcep - adaptive mel-cepstral analysis" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       amcep [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -m m  : order of mel-cepstrum        (   int)[" << std::setw(5) << std::right << kDefaultNumOrder         << "][   0 <= m <=     ]" << std::endl;  // NOLINT
  *stream << "       -a a  : all-pass constant            (double)[" << std::setw(5) << std::right << kDefaultAlpha            << "][     <= a <=     ]" << std::endl;  // NOLINT
  *stream << "       -e e  : minimum value for epsilon    (double)[" << std::setw(5) << std::right << kDefaultMinimumEpsilon   << "][ 0.0 <  e <=     ]" << std::endl;  // NOLINT
  *stream << "       -t t  : momentum constant            (double)[" << std::setw(5) << std::right << kDefaultMomentum         << "][ 0.0 <= t <  1.0 ]" << std::endl;  // NOLINT
  *stream << "       -l l  : forgetting factor            (double)[" << std::setw(5) << std::right << kDefaultForgettingFactor << "][ 0.0 <= l <  1.0 ]" << std::endl;  // NOLINT
  *stream << "       -s s  : step-size factor             (double)[" << std::setw(5) << std::right << kDefaultStepSizeFactor   << "][ 0.0 <  s <  1.0 ]" << std::endl;  // NOLINT
  *stream << "       -p p  : output period                (   int)[" << std::setw(5) << std::right << kDefaultOutputPeriod     << "][   1 <= p <=     ]" << std::endl;  // NOLINT
  *stream << "       -P P  : order of Pade approximation  (   int)[" << std::setw(5) << std::right << kDefaultNumPadeOrder     << "][   4 <= P <= 5   ]" << std::endl;  // NOLINT
  *stream << "       -A    : output averaged mel-cepstrum (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultAverageFlag) << "]" << std::endl;  // NOLINT
  *stream << "       -E E  : output file containing       (string)[" << std::setw(5) << std::right << "N/A"                    << "]" << std::endl;  // NOLINT
  *stream << "               double type prediction error" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence                        (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       mel-cepstrum                         (double)" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);
  double alpha(kDefaultAlpha);
  double minimum_epsilon(kDefaultMinimumEpsilon);
  double momentum(kDefaultMomentum);
  double forgetting_factor(kDefaultForgettingFactor);
  double step_size_factor(kDefaultStepSizeFactor);
  int output_period(kDefaultOutputPeriod);
  int num_pade_order(kDefaultNumPadeOrder);
  bool average_flag(kDefaultAverageFlag);
  const char* prediction_error_file(NULL);

  for (;;) {
    const int option_char(
        getopt_long(argc, argv, "m:a:e:t:l:s:p:P:AE:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("amcep", error_message);
          return 1;
        }
        break;
      }
      case 'a': {
        if (!sptk::ConvertStringToDouble(optarg, &alpha)) {
          std::ostringstream error_message;
          error_message << "The argument for the -a option must be numeric";
          sptk::PrintErrorMessage("amcep", error_message);
          return 1;
        }
        break;
      }
      case 'e': {
        if (!sptk::ConvertStringToDouble(optarg, &minimum_epsilon) ||
            minimum_epsilon <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -e option must be a positive number";
          sptk::PrintErrorMessage("amcep", error_message);
          return 1;
        }
        break;
      }
      case 't': {
        if (!sptk::ConvertStringToDouble(optarg, &momentum) || momentum < 0.0 ||
            1.0 <= momentum) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -t option must be in [0.0, 1.0)";
          sptk::PrintErrorMessage("amcep", error_message);
          return 1;
        }
        break;
      }
      case 'l': {
        if (!sptk::ConvertStringToDouble(optarg, &forgetting_factor) ||
            forgetting_factor < 0.0 || 1.0 <= forgetting_factor) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be in [0.0, 1.0)";
          sptk::PrintErrorMessage("amcep", error_message);
          return 1;
        }
        break;
      }
      case 's': {
        if (!sptk::ConvertStringToDouble(optarg, &step_size_factor) ||
            step_size_factor <= 0.0 || 1.0 <= step_size_factor) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -s option must be in (0.0, 1.0)";
          sptk::PrintErrorMessage("amcep", error_message);
          return 1;
        }
        break;
      }
      case 'p': {
        if (!sptk::ConvertStringToInteger(optarg, &output_period) ||
            output_period <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -p option must be a positive integer";
          sptk::PrintErrorMessage("amcep", error_message);
          return 1;
        }
        break;
      }
      case 'P': {
        const int min(4);
        const int max(5);
        if (!sptk::ConvertStringToInteger(optarg, &num_pade_order) ||
            !sptk::IsInRange(num_pade_order, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -P option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("amcep", error_message);
          return 1;
        }
        break;
      }
      case 'A': {
        average_flag = true;
        break;
      }
      case 'E': {
        prediction_error_file = optarg;
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

  if (average_flag && 1 == output_period) {
    average_flag = false;
  }

  // get input file
  const int num_rest_args(argc - optind);
  if (1 < num_rest_args) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("amcep", error_message);
    return 1;
  }
  const char* input_file(0 == num_rest_args ? NULL : argv[optind]);

  // open input stream for input signal
  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("amcep", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  // open output stream for prediction error
  std::ofstream ofs;
  if (NULL != prediction_error_file) {
    ofs.open(prediction_error_file, std::ios::out | std::ios::binary);
    if (ofs.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << prediction_error_file;
      sptk::PrintErrorMessage("amcep", error_message);
      return 1;
    }
  }
  std::ostream& output_stream(ofs);

  sptk::AdaptiveMelCepstralAnalyzer analyzer(
      num_order, num_pade_order, alpha, minimum_epsilon, momentum,
      forgetting_factor, step_size_factor);
  sptk::AdaptiveMelCepstralAnalyzer::Buffer buffer_for_analyzer;
  if (!analyzer.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to set condition for analyze";
    sptk::PrintErrorMessage("amcep", error_message);
    return 1;
  }
  sptk::StatisticsAccumulator accumulator(num_order, 1);
  sptk::StatisticsAccumulator::Buffer buffer_for_accumulator;

  double input_signal;
  const int length(num_order + 1);
  std::vector<double> mel_cepstrum(length);
  std::vector<double> averaged_mel_cepstrum(length);

  for (int i(1); sptk::ReadStream(&input_signal, &input_stream); ++i) {
    double prediction_error;
    if (!analyzer.Run(input_signal, &prediction_error, &mel_cepstrum,
                      &buffer_for_analyzer)) {
      std::ostringstream error_message;
      error_message << "Failed to run adaptive mel-cepstral analysis";
      sptk::PrintErrorMessage("amcep", error_message);
      return 1;
    }

    if (average_flag) {
      if (!accumulator.Run(mel_cepstrum, &buffer_for_accumulator)) {
        std::ostringstream error_message;
        error_message << "Failed to accumulate mel-cepstrum";
        sptk::PrintErrorMessage("amcep", error_message);
        return 1;
      }
    }

    if (NULL != prediction_error_file) {
      if (!sptk::WriteStream(prediction_error, &output_stream)) {
        std::ostringstream error_message;
        error_message << "Failed to write prediction error";
        sptk::PrintErrorMessage("amcep", error_message);
        return 1;
      }
    }

    if (0 == i % output_period) {
      if (average_flag) {
        if (!accumulator.GetMean(buffer_for_accumulator,
                                 &averaged_mel_cepstrum)) {
          std::ostringstream error_message;
          error_message << "Failed to average mel-cepstrum";
          sptk::PrintErrorMessage("amcep", error_message);
          return 1;
        }
        if (!sptk::WriteStream(0, length, averaged_mel_cepstrum, &std::cout)) {
          std::ostringstream error_message;
          error_message << "Failed to write mel-cepstrum";
          sptk::PrintErrorMessage("amcep", error_message);
          return 1;
        }
        accumulator.Clear(&buffer_for_accumulator);
      } else {
        if (!sptk::WriteStream(0, length, mel_cepstrum, &std::cout)) {
          std::ostringstream error_message;
          error_message << "Failed to write mel-cepstrum";
          sptk::PrintErrorMessage("amcep", error_message);
          return 1;
        }
      }
      i = 0;
    }
  }

  return 0;
}
