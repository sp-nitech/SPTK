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
//                1996-2020  Nagoya Institute of Technology          //
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

#include <fstream>   // std::ifstream, std::ofstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream
#include <vector>    // std::vector

#include "SPTK/analyzer/adaptive_generalized_cepstral_analysis.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultNumOrder(25);
const int kDefaultNumStage(1);
const double kDefaultMinimumEpsilon(1e-16);
const double kDefaultMomentum(0.9);
const double kDefaultForgettingFactor(0.98);
const double kDefaultStepSizeFactor(0.1);
const int kDefaultOutputPeriod(1);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " agcep - adaptive generalized cepstral analysis" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       agcep [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -m m  : order of cepstrum            (   int)[" << std::setw(5) << std::right << kDefaultNumOrder         << "][    0 <= m <=     ]" << std::endl;  // NOLINT
  *stream << "       -c c  : gamma = -1 / c               (   int)[" << std::setw(5) << std::right << kDefaultNumStage         << "][    1 <= c <=     ]" << std::endl;  // NOLINT
  *stream << "       -e e  : minimum value for epsilon    (double)[" << std::setw(5) << std::right << kDefaultMinimumEpsilon   << "][  0.0 <  e <=     ]" << std::endl;  // NOLINT
  *stream << "       -t t  : momentum constant            (double)[" << std::setw(5) << std::right << kDefaultMomentum         << "][  0.0 <= t <  1.0 ]" << std::endl;  // NOLINT
  *stream << "       -l l  : forgetting factor            (double)[" << std::setw(5) << std::right << kDefaultForgettingFactor << "][  0.0 <= l <  1.0 ]" << std::endl;  // NOLINT
  *stream << "       -k k  : step-size factor             (double)[" << std::setw(5) << std::right << kDefaultStepSizeFactor   << "][  0.0 <  s <  1.0 ]" << std::endl;  // NOLINT
  *stream << "       -p p  : output period                (   int)[" << std::setw(5) << std::right << kDefaultOutputPeriod     << "][    1 <= p <=     ]" << std::endl;  // NOLINT
  *stream << "       -E E  : output filename of double    (string)[" << std::setw(5) << std::right << "N/A"                    << "]" << std::endl;  // NOLINT
  *stream << "               type prediction error" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence                        (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       generalized cepstrum                 (double)" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * \a agcep [ \e option ] [ \e infile ]
 *
 * - \b -m \e int
 *   - order of generalized cepstral coefficients \f$(0 \le M)\f$
 * - \b -c \e int
 *   - gamma \f$\gamma = -1 / C\f$ \f$(1 \le C)\f$
 * - \b -e \e double
 *   - minimum epsilon \f$(0 < \epsilon_{min})\f$
 * - \b -t \e double
 *   - momentum \f$(0 \le \tau < 1)\f$
 * - \b -l \e double
 *   - forgetting factor \f$(0 \le \lambda < 1)\f$
 * - \b -k \e double
 *   - step-size factor \f$(0 < a < 1)\f$
 * - \b -p \e int
 *   - output period \f$(1 \le p)\f$
 * - \b -E \e str
 *   - double-type prediction errors
 * - \b infile \e str
 *   - double-type input signals
 * - \b stdout
 *   - double-type generalized cepstral coefficients
 *
 * The below example extracts 15-th order generalized cepstral coefficients for
 * every block of 100 samples.
 *
 * @code{.sh}
 *   agcep -m 15 -p 100 < data.raw > data.gcep
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on false.
 */
int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);
  int num_stage(kDefaultNumStage);
  double minimum_epsilon(kDefaultMinimumEpsilon);
  double momentum(kDefaultMomentum);
  double forgetting_factor(kDefaultForgettingFactor);
  double step_size_factor(kDefaultStepSizeFactor);
  int output_period(kDefaultOutputPeriod);
  const char* prediction_error_file(NULL);

  for (;;) {
    const int option_char(
        getopt_long(argc, argv, "m:c:e:t:l:k:p:E:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("agcep", error_message);
          return 1;
        }
        break;
      }
      case 'c': {
        if (!sptk::ConvertStringToInteger(optarg, &num_stage) ||
            num_stage <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -c option must be a positive integer";
          sptk::PrintErrorMessage("agcep", error_message);
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
          sptk::PrintErrorMessage("agcep", error_message);
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
          sptk::PrintErrorMessage("agcep", error_message);
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
          sptk::PrintErrorMessage("agcep", error_message);
          return 1;
        }
        break;
      }
      case 'k': {
        if (!sptk::ConvertStringToDouble(optarg, &step_size_factor) ||
            step_size_factor <= 0.0 || 1.0 <= step_size_factor) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -k option must be in (0.0, 1.0)";
          sptk::PrintErrorMessage("agcep", error_message);
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
          sptk::PrintErrorMessage("agcep", error_message);
          return 1;
        }
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

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("agcep", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("agcep", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  std::ofstream ofs;
  if (NULL != prediction_error_file) {
    ofs.open(prediction_error_file, std::ios::out | std::ios::binary);
    if (ofs.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << prediction_error_file;
      sptk::PrintErrorMessage("agcep", error_message);
      return 1;
    }
  }
  std::ostream& output_stream(ofs);

  sptk::AdaptiveGeneralizedCepstralAnalysis analysis(
      num_order, num_stage, minimum_epsilon, momentum, forgetting_factor,
      step_size_factor);
  sptk::AdaptiveGeneralizedCepstralAnalysis::Buffer buffer_for_analysis;
  if (!analysis.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize AdaptiveGeneralizedCepstralAnalysis";
    sptk::PrintErrorMessage("agcep", error_message);
    return 1;
  }

  const int length(num_order + 1);
  std::vector<double> generalized_cepstrum(length);
  double input_signal;

  for (int i(1); sptk::ReadStream(&input_signal, &input_stream); ++i) {
    double prediction_error;
    if (!analysis.Run(input_signal, &prediction_error, &generalized_cepstrum,
                      &buffer_for_analysis)) {
      std::ostringstream error_message;
      error_message << "Failed to run adaptive generalized cepstral analysis";
      sptk::PrintErrorMessage("agcep", error_message);
      return 1;
    }

    if (NULL != prediction_error_file) {
      if (!sptk::WriteStream(prediction_error, &output_stream)) {
        std::ostringstream error_message;
        error_message << "Failed to write prediction error";
        sptk::PrintErrorMessage("agcep", error_message);
        return 1;
      }
    }

    if (0 == i % output_period) {
      if (!sptk::WriteStream(0, length, generalized_cepstrum, &std::cout,
                             NULL)) {
        std::ostringstream error_message;
        error_message << "Failed to write generalized cepstrum";
        sptk::PrintErrorMessage("agcep", error_message);
        return 1;
      }
      i = 0;
    }
  }

  return 0;
}
