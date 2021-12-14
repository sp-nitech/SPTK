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

#include <fstream>   // std::ifstream, std::ofstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream
#include <vector>    // std::vector

#include "Getopt/getoptwin.h"
#include "SPTK/analysis/adaptive_mel_generalized_cepstral_analysis.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultNumOrder(25);
const double kDefaultAlpha(0.35);
const int kDefaultNumStage(0);
const double kDefaultMinEpsilon(1e-16);
const double kDefaultMomentum(0.9);
const double kDefaultForgettingFactor(0.98);
const double kDefaultStepSizeFactor(0.1);
const int kDefaultOutputPeriod(1);
const int kDefaultNumPadeOrder(4);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " amgcep - adaptive mel-generalized cepstral analysis" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       amgcep [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -m m  : order of mel-cepstrum        (   int)[" << std::setw(5) << std::right << kDefaultNumOrder         << "][    0 <= m <=     ]" << std::endl;  // NOLINT
  *stream << "       -a a  : all-pass constant            (double)[" << std::setw(5) << std::right << kDefaultAlpha            << "][ -1.0 <  a <  1.0 ]" << std::endl;  // NOLINT
  *stream << "       -c c  : gamma = -1 / c               (   int)[" << std::setw(5) << std::right << kDefaultNumStage         << "][    0 <= c <=     ]" << std::endl;  // NOLINT
  *stream << "       -e e  : minimum value for epsilon    (double)[" << std::setw(5) << std::right << kDefaultMinEpsilon       << "][  0.0 <  e <=     ]" << std::endl;  // NOLINT
  *stream << "       -t t  : momentum constant            (double)[" << std::setw(5) << std::right << kDefaultMomentum         << "][  0.0 <= t <  1.0 ]" << std::endl;  // NOLINT
  *stream << "       -l l  : forgetting factor            (double)[" << std::setw(5) << std::right << kDefaultForgettingFactor << "][  0.0 <= l <  1.0 ]" << std::endl;  // NOLINT
  *stream << "       -k k  : step-size factor             (double)[" << std::setw(5) << std::right << kDefaultStepSizeFactor   << "][  0.0 <  s <  1.0 ]" << std::endl;  // NOLINT
  *stream << "       -p p  : output period                (   int)[" << std::setw(5) << std::right << kDefaultOutputPeriod     << "][    1 <= p <=     ]" << std::endl;  // NOLINT
  *stream << "       -P P  : order of Pade approximation  (   int)[" << std::setw(5) << std::right << kDefaultNumPadeOrder     << "][    4 <= P <= 7   ]" << std::endl;  // NOLINT
  *stream << "       -E E  : output filename of double    (string)[" << std::setw(5) << std::right << "N/A"                    << "]" << std::endl;  // NOLINT
  *stream << "               type prediction error" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence                        (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       mel-generalized cepstrum             (double)" << std::endl;  // NOLINT
  *stream << "  notice:" << std::endl;
  *stream << "       a != 0 and c != 0 is not supported currently" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a amgcep [ @e option ] [ @e infile ]
 *
 * - @b -m @e int
 *   - order of mel-cepstral coefficients @f$(0 \le M)@f$
 * - @b -a @e double
 *   - all-pass constant @f$(|\alpha| < 1)@f$
 * - @b -c @e int
 *   - gamma @f$\gamma = -1 / C@f$ @f$(1 \le C)@f$
 * - @b -e @e double
 *   - minimum epsilon @f$(0 < \epsilon_{min})@f$
 * - @b -t @e double
 *   - momentum @f$(0 \le \tau < 1)@f$
 * - @b -l @e double
 *   - forgetting factor @f$(0 \le \lambda < 1)@f$
 * - @b -k @e double
 *   - step-size factor @f$(0 < a < 1)@f$
 * - @b -p @e int
 *   - output period @f$(1 \le p)@f$
 * - @b -P @e int
 *   - order of Pade approximation @f$(4 \le P \le 7)@f$
 * - @b -E @e str
 *   - double-type prediction errors
 * - @b infile @e str
 *   - double-type input signals
 * - @b stdout
 *   - double-type mel-generalized cepstral coefficients
 *
 * The below example extracts 15-th order mel-cepstral coefficients for every
 * block of 100 samples.
 *
 * @code{.sh}
 *   amgcep -m 15 -p 100 < data.raw > data.mcep
 * @endcode
 *
 * The smoothed mel-cepstral coefficients can be computed as
 *
 * @code{.sh}
 *   amgcep -m 15 -p 1 < data.raw | vstat -m 15 -t 100 -o 1 > data.mcep
 * @endcode
 *
 * 15-th order generalized cepstral coefficients can be obtained as
 *
 * @code{.sh}
 *   amgcep -m 15 -c 1 -a 0 < data.raw > data.gcep
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);
  double alpha(kDefaultAlpha);
  int num_stage(kDefaultNumStage);
  double min_epsilon(kDefaultMinEpsilon);
  double momentum(kDefaultMomentum);
  double forgetting_factor(kDefaultForgettingFactor);
  double step_size_factor(kDefaultStepSizeFactor);
  int output_period(kDefaultOutputPeriod);
  int num_pade_order(kDefaultNumPadeOrder);
  const char* prediction_error_file(NULL);

  for (;;) {
    const int option_char(
        getopt_long(argc, argv, "m:a:c:e:t:l:k:p:P:E:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("amgcep", error_message);
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
          sptk::PrintErrorMessage("amgcep", error_message);
          return 1;
        }
        break;
      }
      case 'c': {
        if (!sptk::ConvertStringToInteger(optarg, &num_stage) ||
            num_stage < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -c option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("amgcep", error_message);
          return 1;
        }
        break;
      }
      case 'e': {
        if (!sptk::ConvertStringToDouble(optarg, &min_epsilon) ||
            min_epsilon <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -e option must be a positive number";
          sptk::PrintErrorMessage("amgcep", error_message);
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
          sptk::PrintErrorMessage("amgcep", error_message);
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
          sptk::PrintErrorMessage("amgcep", error_message);
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
          sptk::PrintErrorMessage("amgcep", error_message);
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
          sptk::PrintErrorMessage("amgcep", error_message);
          return 1;
        }
        break;
      }
      case 'P': {
        const int min(4);
        const int max(7);
        if (!sptk::ConvertStringToInteger(optarg, &num_pade_order) ||
            !sptk::IsInRange(num_pade_order, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -P option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("amgcep", error_message);
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

  if (0.0 != alpha && 0 != num_stage) {
    std::ostringstream error_message;
    error_message
        << "Adaptive mel-generalized cepstral analysis is not supported";
    sptk::PrintErrorMessage("amgcep", error_message);
    return 1;
  }

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("amgcep", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("amgcep", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  std::ofstream ofs;
  if (NULL != prediction_error_file) {
    ofs.open(prediction_error_file, std::ios::out | std::ios::binary);
    if (ofs.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << prediction_error_file;
      sptk::PrintErrorMessage("amgcep", error_message);
      return 1;
    }
  }
  std::ostream& output_stream(ofs);

  sptk::AdaptiveMelGeneralizedCepstralAnalysis analysis(
      num_order, num_pade_order, num_stage, alpha, min_epsilon, momentum,
      forgetting_factor, step_size_factor);
  sptk::AdaptiveMelGeneralizedCepstralAnalysis::Buffer buffer;
  if (!analysis.IsValid()) {
    std::ostringstream error_message;
    error_message
        << "Failed to initialize AdaptiveMelGeneralizedCepstralAnalysis";
    sptk::PrintErrorMessage("amgcep", error_message);
    return 1;
  }

  const int length(num_order + 1);
  std::vector<double> mel_generalized_cepstrum(length);
  double input_signal;

  for (int i(1); sptk::ReadStream(&input_signal, &input_stream); ++i) {
    double prediction_error;
    if (!analysis.Run(input_signal, &prediction_error,
                      &mel_generalized_cepstrum, &buffer)) {
      std::ostringstream error_message;
      error_message
          << "Failed to run adaptive mel-generalized cepstral analysis";
      sptk::PrintErrorMessage("amgcep", error_message);
      return 1;
    }

    if (NULL != prediction_error_file) {
      if (!sptk::WriteStream(prediction_error, &output_stream)) {
        std::ostringstream error_message;
        error_message << "Failed to write prediction error";
        sptk::PrintErrorMessage("amgcep", error_message);
        return 1;
      }
    }

    if (0 == i % output_period) {
      if (!sptk::WriteStream(0, length, mel_generalized_cepstrum, &std::cout,
                             NULL)) {
        std::ostringstream error_message;
        error_message << "Failed to write mel-generalized cepstrum";
        sptk::PrintErrorMessage("amgcep", error_message);
        return 1;
      }
      i = 0;
    }
  }

  return 0;
}
