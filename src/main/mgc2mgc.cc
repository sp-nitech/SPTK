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

#include "SPTK/converter/mel_generalized_cepstrum_to_mel_generalized_cepstrum.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultInputNumOrder(25);
const double kDefaultInputAlpha(0.0);
const double kDefaultInputGamma(0.0);
const bool kDefaultInputNormalizationFlag(false);
const bool kDefaultInputMultiplicationFlag(false);
const int kDefaultOutputNumOrder(25);
const double kDefaultOutputAlpha(0.0);
const double kDefaultOutputGamma(1.0);
const bool kDefaultOutputNormalizationFlag(false);
const bool kDefaultOutputMultiplicationFlag(false);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " mgc2mgc - frequency and generalized cepstral transformation" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       mgc2mgc [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -m m  : order of mel-generalized cepstrum (input)            (   int)[" << std::setw(5) << std::right << kDefaultInputNumOrder  << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -a a  : alpha of mel-generalized cepstrum (input)            (double)[" << std::setw(5) << std::right << kDefaultInputAlpha     << "][   <= a <=   ]" << std::endl;  // NOLINT
  *stream << "       -g g  : gamma of mel-generalized cepstrum (input)            (double)[" << std::setw(5) << std::right << kDefaultInputGamma     << "][   <= g <=   ]" << std::endl;  // NOLINT
  *stream << "       -c c  : gamma of mel-generalized cepstrum = -1 / c (input)   (   int)[" << std::setw(5) << std::right << "N/A"                  << "][ 1 <= c <=   ]" << std::endl;  // NOLINT
  *stream << "       -n    : regard input as normalized mel-generalized cepstrum  (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultInputNormalizationFlag)   << "]" << std::endl;  // NOLINT
  *stream << "       -u    : regard input as multiplied by gamma                  (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultInputMultiplicationFlag)  << "]" << std::endl;  // NOLINT
  *stream << "       -M M  : order of mel-generalized cepstrum (output)           (   int)[" << std::setw(5) << std::right << kDefaultOutputNumOrder << "][ 0 <= M <=   ]" << std::endl;  // NOLINT
  *stream << "       -A A  : alpha of mel-generalized cepstrum (output)           (double)[" << std::setw(5) << std::right << kDefaultOutputAlpha    << "][   <= A <=   ]" << std::endl;  // NOLINT
  *stream << "       -G G  : gamma of mel-generalized cepstrum (output)           (double)[" << std::setw(5) << std::right << kDefaultOutputGamma    << "][   <= G <=   ]" << std::endl;  // NOLINT
  *stream << "       -C C  : gamma of mel-generalized cepstrum = -1 / C (output)  (   int)[" << std::setw(5) << std::right << "N/A"                  << "][ 1 <= C <=   ]" << std::endl;  // NOLINT
  *stream << "       -N    : regard output as normalized mel-generalized cepstrum (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultOutputNormalizationFlag)  << "]" << std::endl;  // NOLINT
  *stream << "       -U    : regard output as multiplied by gamma                 (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultOutputMultiplicationFlag) << "]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       mel-generalized cepstrum                                     (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       transformed mel-generalized cepstrum                         (double)" << std::endl;  // NOLINT
  *stream << "  notice:" << std::endl;
  *stream << "       if -u is used without -n, input is regarded as 1+g*mgc[0],g*mgc[1],...,g*mgc[m]" << std::endl;  // NOLINT
  *stream << "       if -U is used without -N, output is regarded as 1+g*mgc[0],g*mgc[1],...,g*mgc[m]" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

int main(int argc, char* argv[]) {
  int input_num_order(kDefaultInputNumOrder);
  double input_alpha(kDefaultInputAlpha);
  double input_gamma(kDefaultInputGamma);
  bool input_normalization_flag(kDefaultInputNormalizationFlag);
  bool input_multiplication_flag(kDefaultInputMultiplicationFlag);
  int output_num_order(kDefaultOutputNumOrder);
  double output_alpha(kDefaultOutputAlpha);
  double output_gamma(kDefaultOutputGamma);
  bool output_normalization_flag(kDefaultOutputNormalizationFlag);
  bool output_multiplication_flag(kDefaultOutputMultiplicationFlag);

  for (;;) {
    const int option_char(
        getopt_long(argc, argv, "m:a:g:c:nuM:A:G:C:NUh", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &input_num_order) ||
            input_num_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("mgc2mgc", error_message);
          return 1;
        }
        break;
      }
      case 'a': {
        if (!sptk::ConvertStringToDouble(optarg, &input_alpha)) {
          std::ostringstream error_message;
          error_message << "The argument for the -a option must be numeric";
          sptk::PrintErrorMessage("mgc2mgc", error_message);
          return 1;
        }
        break;
      }
      case 'g': {
        if (!sptk::ConvertStringToDouble(optarg, &input_gamma)) {
          std::ostringstream error_message;
          error_message << "The argument for the -g option must be numeric";
          sptk::PrintErrorMessage("mgc2mgc", error_message);
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
          sptk::PrintErrorMessage("mgc2mgc", error_message);
          return 1;
        }
        input_gamma = -1.0 / tmp;
        break;
      }
      case 'n': {
        input_normalization_flag = true;
        break;
      }
      case 'u': {
        input_multiplication_flag = true;
        break;
      }
      case 'M': {
        if (!sptk::ConvertStringToInteger(optarg, &output_num_order) ||
            output_num_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -M option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("mgc2mgc", error_message);
          return 1;
        }
        break;
      }
      case 'A': {
        if (!sptk::ConvertStringToDouble(optarg, &output_alpha)) {
          std::ostringstream error_message;
          error_message << "The argument for the -A option must be numeric";
          sptk::PrintErrorMessage("mgc2mgc", error_message);
          return 1;
        }
        break;
      }
      case 'G': {
        if (!sptk::ConvertStringToDouble(optarg, &output_gamma)) {
          std::ostringstream error_message;
          error_message << "The argument for the -G option must be numeric";
          sptk::PrintErrorMessage("mgc2mgc", error_message);
          return 1;
        }
        break;
      }
      case 'C': {
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) || tmp < 1) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -C option must be a positive integer";
          sptk::PrintErrorMessage("mgc2mgc", error_message);
          return 1;
        }
        output_gamma = -1.0 / tmp;
        break;
      }
      case 'N': {
        output_normalization_flag = true;
        break;
      }
      case 'U': {
        output_multiplication_flag = true;
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
    sptk::PrintErrorMessage("mgc2mgc", error_message);
    return 1;
  }
  const char* input_file(0 == num_rest_args ? NULL : argv[optind]);

  // open stream
  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("mgc2mgc", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  // prepare for gain normalization
  sptk::MelGeneralizedCepstrumToMelGeneralizedCepstrum
      mel_generalized_cepstrum_transform(
          input_num_order, input_alpha, input_gamma, input_normalization_flag,
          input_multiplication_flag, output_num_order, output_alpha,
          output_gamma, output_normalization_flag, output_multiplication_flag);
  sptk::MelGeneralizedCepstrumToMelGeneralizedCepstrum::Buffer buffer;
  if (!mel_generalized_cepstrum_transform.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to set the condition";
    sptk::PrintErrorMessage("mgc2mgc", error_message);
    return 1;
  }

  const int input_length(input_num_order + 1);
  const int output_length(output_num_order + 1);
  std::vector<double> mel_generalized_cepstrum(input_length);
  std::vector<double> transformed_mel_generalized_cepstrum(output_length);

  while (sptk::ReadStream(false, 0, 0, input_length, &mel_generalized_cepstrum,
                          &input_stream)) {
    // input modification: 1+g*mgc[0] -> mgc[0]
    if (!input_normalization_flag && input_multiplication_flag)
      (*mel_generalized_cepstrum.begin()) =
          (*(mel_generalized_cepstrum.begin()) - 1.0) / input_gamma;
    // transform
    if (!mel_generalized_cepstrum_transform.Run(
            mel_generalized_cepstrum, &transformed_mel_generalized_cepstrum,
            &buffer)) {
      std::ostringstream error_message;
      error_message << "Failed to run mel-generalized cepstral transformation";
      sptk::PrintErrorMessage("mgc2mgc", error_message);
      return 1;
    }
    // output modification: mgc[0] -> 1+g*mgc[0]
    if (!output_normalization_flag && output_multiplication_flag)
      (*transformed_mel_generalized_cepstrum.begin()) =
          *(transformed_mel_generalized_cepstrum.begin()) * output_gamma + 1.0;
    // write results
    if (!sptk::WriteStream(0, output_length,
                           transformed_mel_generalized_cepstrum, &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write mel-generalized cepstrum";
      sptk::PrintErrorMessage("mgc2mgc", error_message);
      return 1;
    }
  }

  return 0;
}
