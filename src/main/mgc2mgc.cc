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

#include <fstream>   // std::ifstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream
#include <vector>    // std::vector

#include "Getopt/getoptwin.h"
#include "SPTK/conversion/mel_generalized_cepstrum_to_mel_generalized_cepstrum.h"
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
  *stream << "       -m m  : order of mel-generalized cepstrum (input)            (   int)[" << std::setw(5) << std::right << kDefaultInputNumOrder  << "][    0 <= m <=     ]" << std::endl;  // NOLINT
  *stream << "       -a a  : alpha of mel-generalized cepstrum (input)            (double)[" << std::setw(5) << std::right << kDefaultInputAlpha     << "][ -1.0 <  a <  1.0 ]" << std::endl;  // NOLINT
  *stream << "       -g g  : gamma of mel-generalized cepstrum (input)            (double)[" << std::setw(5) << std::right << kDefaultInputGamma     << "][ -1.0 <= g <= 1.0 ]" << std::endl;  // NOLINT
  *stream << "       -c c  : gamma of mel-generalized cepstrum = -1 / c (input)   (   int)[" << std::setw(5) << std::right << "N/A"                  << "][    1 <= c <=     ]" << std::endl;  // NOLINT
  *stream << "       -n    : regard input as normalized mel-generalized cepstrum  (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultInputNormalizationFlag)   << "]" << std::endl;  // NOLINT
  *stream << "       -u    : regard input as multiplied by gamma                  (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultInputMultiplicationFlag)  << "]" << std::endl;  // NOLINT
  *stream << "       -M M  : order of mel-generalized cepstrum (output)           (   int)[" << std::setw(5) << std::right << kDefaultOutputNumOrder << "][    0 <= M <=     ]" << std::endl;  // NOLINT
  *stream << "       -A A  : alpha of mel-generalized cepstrum (output)           (double)[" << std::setw(5) << std::right << kDefaultOutputAlpha    << "][ -1.0 <  A <  1.0 ]" << std::endl;  // NOLINT
  *stream << "       -G G  : gamma of mel-generalized cepstrum (output)           (double)[" << std::setw(5) << std::right << kDefaultOutputGamma    << "][ -1.0 <= G <= 1.0 ]" << std::endl;  // NOLINT
  *stream << "       -C C  : gamma of mel-generalized cepstrum = -1 / C (output)  (   int)[" << std::setw(5) << std::right << "N/A"                  << "][    1 <= C <=     ]" << std::endl;  // NOLINT
  *stream << "       -N    : regard output as normalized mel-generalized cepstrum (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultOutputNormalizationFlag)  << "]" << std::endl;  // NOLINT
  *stream << "       -U    : regard output as multiplied by gamma                 (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultOutputMultiplicationFlag) << "]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       mel-generalized cepstrum                                     (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       transformed mel-generalized cepstrum                         (double)" << std::endl;  // NOLINT
  *stream << "  notice:" << std::endl;
  *stream << "       if -u is used without -n, input is regarded as 1+g*mgc[0],g*mgc[1],...,g*mgc[m]" << std::endl;  // NOLINT
  *stream << "       if -U is used without -N, output is regarded as 1+g*mgc[0],g*mgc[1],...,g*mgc[M]" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a mgc2mgc [ @e option ] [ @e infile ]
 *
 * - @b -m @e int
 *   - order of input coefficients @f$(0 \le M_1)@f$
 * - @b -a @e double
 *   - input all-pass constant @f$(|\alpha_1| < 1)@f$
 * - @b -g @e double
 *   - input gamma @f$(|\gamma_1| \le 1)@f$
 * - @b -c @e int
 *   - input gamma @f$\gamma_1 = -1 / C_1@f$ @f$(1 \le C_1)@f$
 * - @b -n @e bool
 *   - regard input as normalized mel-generalized cepstrum
 * - @b -u @e bool
 *   - regard input as multiplied by gamma
 * - @b -M @e int
 *   - order of output coefficients @f$(0 \le M_2)@f$
 * - @b -A @e double
 *   - output all-pass constant @f$(|\alpha_2| < 1)@f$
 * - @b -G @e double
 *   - output gamma @f$(|\gamma_2| \le 1)@f$
 * - @b -C @e int
 *   - output gamma @f$\gamma_2 = -1 / C_2@f$ @f$(1 \le C_2)@f$
 * - @b -N @e bool
 *   - regard output as normalized mel-generalized cepstrum
 * - @b -U @e bool
 *   - regard output as multiplied by gamma
 * - @b infile @e str
 *   - double-type mel-generalized cepstral coefficients
 * - @b stdout
 *   - double-type converted mel-generalized cepstral coefficients
 *
 * If @c -u without @c -n, the 0th input is regarded as
 * @f$1 + \gamma_1 c_{\alpha_1,\gamma_1}(0)@f$.
 * If @c -U without @c -N, the 0th output is regarded as
 * @f$1 + \gamma_2 c_{\alpha_2,\gamma_2}(0)@f$.
 *
 * In the example below, 12-th order LPC coefficients in @c data.lpc are
 * converted to 30-th order mel-cepstral coefficients.
 *
 * @code{.sh}
 *   mgc2mgc -m 12 -a 0 -g -1 -M 30 -A 0.31 -G 0 < data.lpc > data.mcep
 * @endcode
 *
 * Impulse response can be calculated using this command instead of @c c2mpir.
 *
 * @code{.sh}
 *   mgc2mgc -m 10 -a 0.4 -g 0 -M 30 -A 0.0 -G 1 -U < data.mcep > data.ir
 *   # This is equivalent to that:
 *   # freqt -a 0.4 -A 0.0 -m 10 -M 30 data.mcep | c2mpir -m 30 -M 30 > data.ir
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
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
        if (!sptk::ConvertStringToDouble(optarg, &input_alpha) ||
            !sptk::IsValidAlpha(input_alpha)) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -a option must be in (-1.0, 1.0)";
          sptk::PrintErrorMessage("mgc2mgc", error_message);
          return 1;
        }
        break;
      }
      case 'g': {
        if (!sptk::ConvertStringToDouble(optarg, &input_gamma) ||
            !sptk::IsValidGamma(input_gamma)) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -g option must be in [-1.0, 1.0]";
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
        if (!sptk::ConvertStringToDouble(optarg, &output_alpha) ||
            !sptk::IsValidAlpha(output_alpha)) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -A option must be in (-1.0, 1.0)";
          sptk::PrintErrorMessage("mgc2mgc", error_message);
          return 1;
        }
        break;
      }
      case 'G': {
        if (!sptk::ConvertStringToDouble(optarg, &output_gamma) ||
            !sptk::IsValidGamma(output_gamma)) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -G option must be in [-1.0, 1.0]";
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

  if (0.0 == input_gamma && input_multiplication_flag) {
    std::ostringstream error_message;
    error_message << "If -u option is given, input gamma must not be 0";
    sptk::PrintErrorMessage("mgc2mgc", error_message);
    return 1;
  }

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("mgc2mgc", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("mgc2mgc", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::MelGeneralizedCepstrumToMelGeneralizedCepstrum
      mel_generalized_cepstrum_transform(
          input_num_order, input_alpha, input_gamma, input_normalization_flag,
          input_multiplication_flag, output_num_order, output_alpha,
          output_gamma, output_normalization_flag, output_multiplication_flag);
  sptk::MelGeneralizedCepstrumToMelGeneralizedCepstrum::Buffer buffer;
  if (!mel_generalized_cepstrum_transform.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize "
                  << "MelGeneralizedCepstrumToMelGeneralizedCepstrum";
    sptk::PrintErrorMessage("mgc2mgc", error_message);
    return 1;
  }

  const int input_length(input_num_order + 1);
  const int output_length(output_num_order + 1);
  std::vector<double> mel_generalized_cepstrum(input_length);
  std::vector<double> transformed_mel_generalized_cepstrum(output_length);

  while (sptk::ReadStream(false, 0, 0, input_length, &mel_generalized_cepstrum,
                          &input_stream, NULL)) {
    // Perform input modification: 1+g*mgc[0] -> mgc[0]
    if (!input_normalization_flag && input_multiplication_flag)
      (*mel_generalized_cepstrum.begin()) =
          (*(mel_generalized_cepstrum.begin()) - 1.0) / input_gamma;

    // Transform.
    if (!mel_generalized_cepstrum_transform.Run(
            mel_generalized_cepstrum, &transformed_mel_generalized_cepstrum,
            &buffer)) {
      std::ostringstream error_message;
      error_message << "Failed to run mel-generalized cepstral transformation";
      sptk::PrintErrorMessage("mgc2mgc", error_message);
      return 1;
    }

    // Perform output modification: mgc[0] -> 1+g*mgc[0]
    if (!output_normalization_flag && output_multiplication_flag)
      (*transformed_mel_generalized_cepstrum.begin()) =
          *(transformed_mel_generalized_cepstrum.begin()) * output_gamma + 1.0;

    // Write results.
    if (!sptk::WriteStream(0, output_length,
                           transformed_mel_generalized_cepstrum, &std::cout,
                           NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write mel-generalized cepstrum";
      sptk::PrintErrorMessage("mgc2mgc", error_message);
      return 1;
    }
  }

  return 0;
}
