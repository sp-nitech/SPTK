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
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "SPTK/utils/mlsa_digital_filter_stability_check.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum WarningType { kIgnore = 0, kWarn, kExit, kNumWarningTypes };

const int kDefaultNumFilterOrder(25);
const int kDefaultFftLength(256);
const double kDefaultAlpha(0.35);
const int kDefaultNumPadeOrder(4);
const WarningType kDefaultWarningType(kWarn);
const bool kDefaultKeepMaximumLogApproximationErrorFlag(true);
const bool kDefaultFastModeFlag(false);
const bool kDefaultModificationFlag(false);
const sptk::MlsaDigitalFilterStabilityCheck::ModificationType
    kDefaultModificationType(
        sptk::MlsaDigitalFilterStabilityCheck::ModificationType::kClipping);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " mlsacheck - check stability of MLSA digital filter" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       mlsacheck [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -m m  : order of filter coefficients      (   int)[" << std::setw(5) << std::right << kDefaultNumFilterOrder   << "][   0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -l l  : FFT length                        (   int)[" << std::setw(5) << std::right << kDefaultFftLength        << "][   m <  l <=   ]" << std::endl;  // NOLINT
  *stream << "       -a a  : all-pass constant                 (double)[" << std::setw(5) << std::right << kDefaultAlpha            << "][     <= a <=   ]" << std::endl;  // NOLINT
  *stream << "       -P P  : order of Pade approximation       (   int)[" << std::setw(5) << std::right << kDefaultNumPadeOrder     << "][   4 <= P <= 7 ]" << std::endl;  // NOLINT
  *stream << "       -e e  : warning type of unstable index    (   int)[" << std::setw(5) << std::right << kDefaultWarningType      << "][   0 <= e <= 2 ]" << std::endl;  // NOLINT
  *stream << "                 0 (no warning)" << std::endl;
  *stream << "                 1 (output the index to stderr)" << std::endl;
  *stream << "                 2 (output the index to stderr" << std::endl;
  *stream << "                    and exit immediately)" << std::endl;
  *stream << "       -r r  : threshold value                   (double)[" << std::setw(5) << std::right << "N/A"                    << "][ 0.0 <  r <=   ]" << std::endl;  // NOLINT
  *stream << "       -k    : keep filter stability rather than (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(!kDefaultKeepMaximumLogApproximationErrorFlag) << "]" << std::endl;  // NOLINT
  *stream << "               maximum log approximation error" << std::endl;
  *stream << "       -f    : fast mode                         (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultFastModeFlag)                          << "]" << std::endl;  // NOLINT
  *stream << "       -x    : perform modification              (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultModificationFlag)                      << "]" << std::endl;  // NOLINT
  *stream << "       -t    : modification type                 (   int)[" << std::setw(5) << std::right << kDefaultModificationType << "][   0 <= t <= 1 ]" << std::endl;  // NOLINT
  *stream << "                 0 (clipping)" << std::endl;
  *stream << "                 1 (scaling)" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       mel-cepstrum                              (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       modified mel-cepstrum                     (double)" << std::endl;  // NOLINT
  *stream << "  notice:" << std::endl;
  *stream << "       value of l must be a power of 2" << std::endl;
  *stream << "       if -r option is not specified, an appropriate threshold value is used according to -k and -P options" << std::endl;  // NOLINT
  *stream << "       -t option is valid only if -f option is not specified" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

int main(int argc, char* argv[]) {
  int num_filter_order(kDefaultNumFilterOrder);
  int fft_length(kDefaultFftLength);
  double alpha(kDefaultAlpha);
  int num_pade_order(kDefaultNumPadeOrder);
  WarningType warning_type(kDefaultWarningType);
  double threshold(0.0);
  bool keep_maximum_log_approximation_error_flag(
      kDefaultKeepMaximumLogApproximationErrorFlag);
  bool fast_mode_flag(kDefaultFastModeFlag);
  bool modification_flag(kDefaultModificationFlag);
  sptk::MlsaDigitalFilterStabilityCheck::ModificationType modification_type(
      kDefaultModificationType);

  for (;;) {
    const int option_char(
        getopt_long(argc, argv, "m:l:a:P:e:r:kfxt:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_filter_order) ||
            num_filter_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("mlsacheck", error_message);
          return 1;
        }
        break;
      }
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &fft_length)) {
          std::ostringstream error_message;
          error_message << "The argument for the -l option must be an integer";
          sptk::PrintErrorMessage("mlsacheck", error_message);
          return 1;
        }
        break;
      }
      case 'a': {
        if (!sptk::ConvertStringToDouble(optarg, &alpha)) {
          std::ostringstream error_message;
          error_message << "The argument for the -a option must be numeric";
          sptk::PrintErrorMessage("mlsacheck", error_message);
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
          sptk::PrintErrorMessage("mlsacheck", error_message);
          return 1;
        }
        break;
      }
      case 'e': {
        const int min(0);
        const int max(static_cast<int>(kNumWarningTypes) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -e option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("mlsacheck", error_message);
          return 1;
        }
        warning_type = static_cast<WarningType>(tmp);
        break;
      }
      case 'r': {
        if (!sptk::ConvertStringToDouble(optarg, &threshold) ||
            threshold <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -r option must be a positive number";
          sptk::PrintErrorMessage("mlsacheck", error_message);
          return 1;
        }
        break;
      }
      case 'k': {
        keep_maximum_log_approximation_error_flag = false;
        break;
      }
      case 'f': {
        fast_mode_flag = true;
        break;
      }
      case 'x': {
        modification_flag = true;
        break;
      }
      case 't': {
        const int min(0);
        const int max(
            static_cast<int>(sptk::MlsaDigitalFilterStabilityCheck::
                                 ModificationType::kNumModificationTypes) -
            1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -t option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("mlsacheck", error_message);
          return 1;
        }
        modification_type = static_cast<
            sptk::MlsaDigitalFilterStabilityCheck::ModificationType>(tmp);
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

  if (!fast_mode_flag && fft_length <= num_filter_order) {
    std::ostringstream error_message;
    error_message
        << "Order of filter coefficients must be less than FFT length";
    sptk::PrintErrorMessage("mlsacheck", error_message);
    return 1;
  }

  if (0.0 == threshold) {
    if (4 == num_pade_order) {
      threshold = keep_maximum_log_approximation_error_flag ? 4.5 : 6.20;
    } else if (5 == num_pade_order) {
      threshold = keep_maximum_log_approximation_error_flag ? 6.0 : 7.65;
    } else if (6 == num_pade_order) {
      threshold = keep_maximum_log_approximation_error_flag ? 7.4 : 9.13;
    } else if (7 == num_pade_order) {
      threshold = keep_maximum_log_approximation_error_flag ? 8.9 : 10.6;
    }
  }

  // get input file
  const int num_rest_args(argc - optind);
  if (1 < num_rest_args) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("mlsacheck", error_message);
    return 1;
  }
  const char* input_file(0 == num_rest_args ? NULL : argv[optind]);

  // open stream
  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("mlsacheck", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  // prepare for stability check
  sptk::MlsaDigitalFilterStabilityCheck mlsa_digital_filter_stability_check(
      num_filter_order, alpha, threshold, fast_mode_flag, fft_length,
      modification_type);
  sptk::MlsaDigitalFilterStabilityCheck::Buffer buffer;
  if (!mlsa_digital_filter_stability_check.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to set condition for stability check";
    sptk::PrintErrorMessage("mlsacheck", error_message);
    return 1;
  }

  const int length(num_filter_order + 1);
  std::vector<double> input_mel_cepstrum(length);
  std::vector<double> output_mel_cepstrum(length);
  std::vector<double>* output_ptr(modification_flag ? &output_mel_cepstrum
                                                    : NULL);
  std::vector<double>* write_ptr(modification_flag ? &output_mel_cepstrum
                                                   : &input_mel_cepstrum);

  for (int frame_index(0); sptk::ReadStream(
           false, 0, 0, length, &input_mel_cepstrum, &input_stream, NULL);
       ++frame_index) {
    bool is_stable(false);
    double maximum_amplitude(0.0);
    if (!mlsa_digital_filter_stability_check.Run(input_mel_cepstrum, output_ptr,
                                                 &is_stable, &maximum_amplitude,
                                                 &buffer)) {
      std::ostringstream error_message;
      error_message << "Failed to check stability of MLSA digital filter";
      sptk::PrintErrorMessage("mlsacheck", error_message);
      return 1;
    }

    if (!is_stable && kIgnore != warning_type) {
      std::ostringstream error_message;
      error_message << frame_index << "th frame is unstable ("
                    << "maximum = " << maximum_amplitude << ", "
                    << "threshold = " << threshold << ")";
      sptk::PrintErrorMessage("mlsacheck", error_message);
      if (kExit == warning_type) return 1;
    }

    if (!sptk::WriteStream(0, length, *write_ptr, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write mel-cepstrum";
      sptk::PrintErrorMessage("mlsacheck", error_message);
      return 1;
    }
  }

  return 0;
}
