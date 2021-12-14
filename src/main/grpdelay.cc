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
#include "SPTK/conversion/filter_coefficients_to_group_delay.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultFftLength(256);
const int kDefaultNumNumeratorOrder(0);
const int kDefaultNumDenominatorOrder(0);
const double kDefaultAlpha(1.0);
const double kDefaultGamma(1.0);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " grpdelay - transform real sequence to group delay" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       grpdelay [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : FFT length                        (   int)[" << std::setw(5) << std::right << kDefaultFftLength           << "][   2 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : order of numerator coefficients   (   int)[" << std::setw(5) << std::right << kDefaultNumNumeratorOrder   << "][   0 <= m <  l ]" << std::endl;  // NOLINT
  *stream << "       -n n  : order of denominator coefficients (   int)[" << std::setw(5) << std::right << kDefaultNumDenominatorOrder << "][   0 <= n <  l ]" << std::endl;  // NOLINT
  *stream << "       -z z  : name of file containing           (string)[" << std::setw(5) << std::right << "N/A"                       << "]" << std::endl;  // NOLINT
  *stream << "               numerator coefficients" << std::endl;
  *stream << "       -p p  : name of file containing           (string)[" << std::setw(5) << std::right << "N/A"                       << "]" << std::endl;  // NOLINT
  *stream << "               denominator coefficients" << std::endl;
  *stream << "       -a a  : alpha of MGDF                     (double)[" << std::setw(5) << std::right << kDefaultAlpha               << "][ 0.0 <  a <=   ]" << std::endl;  // NOLINT
  *stream << "       -g g  : gamma of MGDF                     (double)[" << std::setw(5) << std::right << kDefaultGamma               << "][ 0.0 <  g <=   ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence                             (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       group delay                               (double)" << std::endl;  // NOLINT
  *stream << "  notice:" << std::endl;
  *stream << "       value of l must be a power of 2" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a grpdelay [ @e option ] [ @e infile ]
 *
 * - @b -l @e int
 *   - FFT length @f$(2 \le L)@f$
 * - @b -m @e int
 *   - order of numerator coefficients @f$(0 \le M < L)@f$
 * - @b -n @e int
 *   - order of denominator coefficients @f$(0 \le N < L)@f$
 * - @b -z @e str
 *   - name of file containing numerator coefficients
 * - @b -p @e str
 *   - name of file containing denominator coefficients
 * - @b -a @e double
 *   - alpha
 * - @b -g @e double
 *   - gamma
 * - @b infile @e str
 *   - double-type real sequence
 * - @b stdout
 *   - double-type group delay
 *
 * The below example calculates group delay from 10-th order filter
 * coefficients:
 *
 * @code{.sh}
 *   grpdelay -z data.z -m 10 -p data.p -n 10 -l 16 > data.gd
 * @endcode
 *
 * If the filter coefficients are stable, the below example gives the same
 * result:
 *
 * @code{.sh}
 *   impulse -l 16 | dfs -z data.z -p data.p | grpdelay -l 16 > data.gd
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int fft_length(kDefaultFftLength);
  int num_numerator_order(kDefaultNumNumeratorOrder);
  int num_denominator_order(kDefaultNumDenominatorOrder);
  const char* numerator_coefficients_file(NULL);
  const char* denominator_coefficients_file(NULL);
  bool is_numerator_specified(false);
  bool is_denominator_specified(false);
  double alpha(kDefaultAlpha);
  double gamma(kDefaultGamma);

  for (;;) {
    const int option_char(
        getopt_long(argc, argv, "l:m:n:z:p:a:g:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &fft_length)) {
          std::ostringstream error_message;
          error_message << "The argument for the -l option must be an integer";
          sptk::PrintErrorMessage("grpdelay", error_message);
          return 1;
        }
        break;
      }
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_numerator_order) ||
            num_numerator_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                           "non-negative integer";
          sptk::PrintErrorMessage("grpdelay", error_message);
          return 1;
        }
        is_numerator_specified = true;
        break;
      }
      case 'n': {
        if (!sptk::ConvertStringToInteger(optarg, &num_denominator_order) ||
            num_denominator_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -n option must be a "
                           "non-negative integer";
          sptk::PrintErrorMessage("grpdelay", error_message);
          return 1;
        }
        is_denominator_specified = true;
        break;
      }
      case 'z': {
        numerator_coefficients_file = optarg;
        is_numerator_specified = true;
        break;
      }
      case 'p': {
        denominator_coefficients_file = optarg;
        is_denominator_specified = true;
        break;
      }
      case 'a': {
        if (!sptk::ConvertStringToDouble(optarg, &alpha) || alpha <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -a option must be a positive number";
          sptk::PrintErrorMessage("grpdelay", error_message);
          return 1;
        }
        break;
      }
      case 'g': {
        if (!sptk::ConvertStringToDouble(optarg, &gamma) || gamma <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -g option must be a positive number";
          sptk::PrintErrorMessage("grpdelay", error_message);
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

  if (is_numerator_specified || is_denominator_specified) {
    const int numerator_length(num_numerator_order + 1);
    const int denominator_length(num_denominator_order + 1);
    std::vector<double> numerator_coefficients(numerator_length);
    std::vector<double> denominator_coefficients(denominator_length);

    if (is_numerator_specified && is_denominator_specified &&
        (NULL == numerator_coefficients_file ||
         NULL == denominator_coefficients_file)) {
      std::ostringstream error_message;
      error_message
          << "Numerator and denominator coefficient files are required";
      sptk::PrintErrorMessage("grpdelay", error_message);
      return 1;
    }

    const int num_input_files(argc - optind);
    if (0 < num_input_files) {
      std::ostringstream error_message;
      error_message << "Too many input files";
      sptk::PrintErrorMessage("grpdelay", error_message);
      return 1;
    }

    std::ifstream ifs_for_numerator;
    if (is_numerator_specified) {
      ifs_for_numerator.open(numerator_coefficients_file,
                             std::ios::in | std::ios::binary);
      if (ifs_for_numerator.fail() && NULL != numerator_coefficients_file) {
        std::ostringstream error_message;
        error_message << "Cannot open file " << numerator_coefficients_file;
        sptk::PrintErrorMessage("grpdelay", error_message);
        return 1;
      }
    } else {
      numerator_coefficients[0] = 1.0;
    }
    std::istream& input_stream_for_numerator(
        (is_numerator_specified && ifs_for_numerator.fail())
            ? std::cin
            : ifs_for_numerator);

    std::ifstream ifs_for_denominator;
    if (is_denominator_specified) {
      ifs_for_denominator.open(denominator_coefficients_file,
                               std::ios::in | std::ios::binary);
      if (ifs_for_denominator.fail() && NULL != denominator_coefficients_file) {
        std::ostringstream error_message;
        error_message << "Cannot open file " << denominator_coefficients_file;
        sptk::PrintErrorMessage("grpdelay", error_message);
        return 1;
      }
    } else {
      denominator_coefficients[0] = 1.0;
    }
    std::istream& input_stream_for_denominator(
        (is_denominator_specified && ifs_for_denominator.fail())
            ? std::cin
            : ifs_for_denominator);

    sptk::FilterCoefficientsToGroupDelay filter_coefficients_to_group_delay(
        num_numerator_order, num_denominator_order, fft_length, alpha, gamma);
    sptk::FilterCoefficientsToGroupDelay::Buffer buffer;
    if (!filter_coefficients_to_group_delay.IsValid()) {
      std::ostringstream error_message;
      error_message << "Failed to initialize FilterCoefficientsToGroupDelay";
      sptk::PrintErrorMessage("grpdelay", error_message);
      return 1;
    }

    const int output_length(fft_length / 2 + 1);
    std::vector<double> output(output_length);
    while ((!is_numerator_specified ||
            sptk::ReadStream(false, 0, 0, numerator_length,
                             &numerator_coefficients,
                             &input_stream_for_numerator, NULL)) &&
           (!is_denominator_specified ||
            sptk::ReadStream(false, 0, 0, denominator_length,
                             &denominator_coefficients,
                             &input_stream_for_denominator, NULL))) {
      if (!filter_coefficients_to_group_delay.Run(numerator_coefficients,
                                                  denominator_coefficients,
                                                  &output, &buffer)) {
        std::ostringstream error_message;
        error_message
            << "Failed to transform filter coefficients to group delay";
        sptk::PrintErrorMessage("grpdelay", error_message);
        return 1;
      }

      if (!sptk::WriteStream(0, output_length, output, &std::cout, NULL)) {
        std::ostringstream error_message;
        error_message << "Failed to write group delay";
        sptk::PrintErrorMessage("grpdelay", error_message);
        return 1;
      }
    }
  } else {
    const int num_input_files(argc - optind);
    if (1 < num_input_files) {
      std::ostringstream error_message;
      error_message << "Too many input files";
      sptk::PrintErrorMessage("grpdelay", error_message);
      return 1;
    }
    const char* input_file(0 == num_input_files ? NULL : argv[optind]);

    std::ifstream ifs;
    ifs.open(input_file, std::ios::in | std::ios::binary);
    if (ifs.fail() && NULL != input_file) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << input_file;
      sptk::PrintErrorMessage("grpdelay", error_message);
      return 1;
    }
    std::istream& input_stream(ifs.fail() ? std::cin : ifs);

    sptk::FilterCoefficientsToGroupDelay waveform_to_group_delay(
        fft_length - 1, 0, fft_length, alpha, gamma);
    sptk::FilterCoefficientsToGroupDelay::Buffer buffer;
    if (!waveform_to_group_delay.IsValid()) {
      std::ostringstream error_message;
      error_message << "Failed to initialize FilterCoefficientsToGroupDelay";
      sptk::PrintErrorMessage("grpdelay", error_message);
      return 1;
    }

    const int output_length(fft_length / 2 + 1);
    std::vector<double> waveform(fft_length);
    std::vector<double> output(output_length);
    while (sptk::ReadStream(true, 0, 0, fft_length, &waveform, &input_stream,
                            NULL)) {
      if (!waveform_to_group_delay.Run(waveform, {1}, &output, &buffer)) {
        std::ostringstream error_message;
        error_message << "Failed to transform waveform to group delay";
        sptk::PrintErrorMessage("grpdelay", error_message);
        return 1;
      }

      if (!sptk::WriteStream(0, output_length, output, &std::cout, NULL)) {
        std::ostringstream error_message;
        error_message << "Failed to write group delay";
        sptk::PrintErrorMessage("grpdelay", error_message);
        return 1;
      }
    }
  }

  return 0;
}
