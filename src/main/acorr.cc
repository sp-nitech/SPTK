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

#include <getopt.h>  // getopt_long

#include <algorithm>  // std::transform
#include <fstream>    // std::ifstream
#include <iomanip>    // std::setw
#include <iostream>   // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>    // std::ostringstream
#include <vector>     // std::vector

#include "SPTK/conversion/waveform_to_autocorrelation.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum OutputFormats {
  kAutocorrelation = 0,
  kBiasedAutocorrelation,
  kNormalizedAutocorrelation,
  kNumOutputFormats
};

const int kDefaultFrameLength(256);
const int kDefaultNumOrder(25);
const OutputFormats kDefaultOutputFormat(kAutocorrelation);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " acorr - obtain autocorrelation sequence" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       acorr [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : frame length       (   int)[" << std::setw(5) << std::right << kDefaultFrameLength  << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : order of sequence  (   int)[" << std::setw(5) << std::right << kDefaultNumOrder     << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -o o  : output format      (   int)[" << std::setw(5) << std::right << kDefaultOutputFormat << "][ 0 <= o <= 1 ]" << std::endl;  // NOLINT
  *stream << "                 0 (autocorrelation)" << std::endl;
  *stream << "                 1 (biased autocorrelation)" << std::endl;
  *stream << "                 2 (normalized autocorrelation)" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence              (double)[stdin]" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       autocorrelation sequence   (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a acorr [ @e option ] [ @e infile ]
 *
 * - @b -l @e int
 *   - frame length @f$(1 \le L)@f$
 * - @b -m @e int
 *   - order of autocorrelation coefficients @f$(0 \le M)@f$
 * - @b -o @e double
 *   - output format
 *     \arg @c 0 autocorrelation
 *     \arg @c 1 biased autocorrelation
 *     \arg @c 2 normalized autocorrelation
 * - @b infile @e str
 *   - double-type data sequence
 * - @b stdout
 *   - double-type autocorrelation sequence.
 *
 * If `-o 1`, output the biased autocorrelation:
 * @f[
 *   \begin{array}{cccc}
 *     r(0)/L, & r(1)/L, & \ldots, & r(M)/L,
 *   \end{array}
 * @f]
 * where @f$r(m)@f$ is the @f$m@f$-th autocorrelation coefficient.
 * If `-o 2`, output the normalized autocorrelation:
 * @f[
 *   \begin{array}{cccc}
 *     1, & r(1)/r(0), & \ldots, & r(M)/r(0).
 *   \end{array}
 * @f]
 *
 * The below example extracts 10-th order autocorrelation coefficients from
 * windowed waveform.
 *
 * @code{.sh}
 *   x2x +sd data.short | frame | window | acorr -m 10 > data.acr
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int frame_length(kDefaultFrameLength);
  int num_order(kDefaultNumOrder);
  OutputFormats output_format(kDefaultOutputFormat);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:o:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &frame_length) ||
            frame_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("acorr", error_message);
          return 1;
        }
        break;
      }
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("acorr", error_message);
          return 1;
        }
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
          sptk::PrintErrorMessage("acorr", error_message);
          return 1;
        }
        output_format = static_cast<OutputFormats>(tmp);
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
    sptk::PrintErrorMessage("acorr", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("acorr", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::WaveformToAutocorrelation waveform_to_autocorrelation(frame_length,
                                                              num_order);
  if (!waveform_to_autocorrelation.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize WaveformToAutocorrelation";
    sptk::PrintErrorMessage("acorr", error_message);
    return 1;
  }

  const int output_length(num_order + 1);
  std::vector<double> waveform(frame_length);
  std::vector<double> autocorrelation(output_length);

  while (sptk::ReadStream(false, 0, 0, frame_length, &waveform, &input_stream,
                          NULL)) {
    if (!waveform_to_autocorrelation.Run(waveform, &autocorrelation)) {
      std::ostringstream error_message;
      error_message << "Failed to calculate autocorrelation";
      sptk::PrintErrorMessage("acorr", error_message);
      return 1;
    }

    if (kBiasedAutocorrelation == output_format) {
      const double z(1.0 / frame_length);
      std::transform(autocorrelation.begin(), autocorrelation.end(),
                     autocorrelation.begin(), [z](double r) { return r * z; });
    } else if (kNormalizedAutocorrelation == output_format) {
      if (0.0 == autocorrelation[0]) {
        std::ostringstream error_message;
        error_message << "Failed to normalize autocorrelation";
        sptk::PrintErrorMessage("acorr", error_message);
        return 1;
      }

      const double z(1.0 / autocorrelation[0]);
      std::transform(autocorrelation.begin(), autocorrelation.end(),
                     autocorrelation.begin(), [z](double r) { return r * z; });
    }

    if (!sptk::WriteStream(0, output_length, autocorrelation, &std::cout,
                           NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write autocorrelation";
      sptk::PrintErrorMessage("acorr", error_message);
      return 1;
    }
  }

  return 0;
}
