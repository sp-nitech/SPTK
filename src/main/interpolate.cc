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
#include "SPTK/utils/sptk_utils.h"

namespace {

enum OutputFormats { kPadWithZero = 0, kPadWithSameValue, kNumOutputFormats };

const int kDefaultStartIndex(0);
const int kDefaultVectorLength(1);
const int kDefaultInterpolationPeriod(10);
const OutputFormats kDefaultOutputFormat(kPadWithZero);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " interpolate - data interpolation" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       interpolate [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : length of vector     (   int)[" << std::setw(5) << std::right << kDefaultVectorLength        << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : order of vector      (   int)[" << std::setw(5) << std::right << "l-1"                       << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -s s  : start index          (   int)[" << std::setw(5) << std::right << kDefaultStartIndex          << "][ 0 <= s <=   ]" << std::endl;  // NOLINT
  *stream << "       -p p  : interpolation period (   int)[" << std::setw(5) << std::right << kDefaultInterpolationPeriod << "][ 1 <= p <=   ]" << std::endl;  // NOLINT
  *stream << "       -o o  : output format        (   int)[" << std::setw(5) << std::right << kDefaultOutputFormat        << "][ 0 <= o <= 1 ]" << std::endl;  // NOLINT
  *stream << "                 0 ( x(0), 0,    ..., x(1), 0,    ..., )" << std::endl;  // NOLINT
  *stream << "                 1 ( x(0), x(0), ..., x(1), x(1), ..., )" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence                (double)[stdin]" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       interpolated data sequence   (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a interpolate [ @e option ] [ @e infile ]
 *
 * - @b -l @e int
 *   - length of vector @f$(1 \le L)@f$
 * - @b -m @e int
 *   - order of vector @f$(0 \le M)@f$
 * - @b -s @e int
 *   - start index @f$(0 \le S)@f$
 * - @b -p @e int
 *   - interpolation period @f$(1 \le P)@f$
 * - @b -o @e int
 *   - output format @f$(0 \le O \le 1)@f$
 *     \arg @c 0 zero-padding
 *     \arg @c 1 repetition
 * - @b infile @e str
 *   - double-type data sequence
 * - @b stdout
 *   - double-type interpolated data sequence
 *
 * The input of the command is a sequence of @f$L@f$-dimensional vectors:
 * @f[
 *   \begin{array}{cccc}
 *     \boldsymbol{x}(0), & \boldsymbol{x}(1), & \boldsymbol{x}(2), & \ldots,
 *   \end{array}
 * @f]
 * where @f$L=M+1@f$. If @f$O=0@f$, the output is a zero-padded sequence:
 * @f[
 *   \begin{array}{cccc}
 *     \underbrace{\boldsymbol{0},\,\boldsymbol{0},\,\ldots,\,
 *                 \boldsymbol{0}}_{S}, &
 *     \underbrace{\boldsymbol{x}(0),\,\boldsymbol{0},\,\ldots,\,
 *                 \boldsymbol{0}}_{P}, &
 *     \underbrace{\boldsymbol{x}(1),\,\boldsymbol{0},\,\ldots,\,
 *                 \boldsymbol{0}}_{P}, &
 *     \ldots,
 *  \end{array}
 * @f]
 * where @f$\boldsymbol{0}@f$ is the @f$L@f$-dimensional zero vector,
 * @f$S@f$ is the number of left-padded zero vectors, and the @f$P@f$ is the
 * interpolation factor.
 * If @f$O=1@f$, each of the vectors is copied @f$P@f$ times:
 * @f[
 *   \begin{array}{cccc}
 *     \underbrace{\boldsymbol{0},\,\boldsymbol{0},\,\ldots,\,
 *                 \boldsymbol{0}}_{S}, &
 *     \underbrace{\boldsymbol{x}(0),\,\boldsymbol{x}(0),\,\ldots,\,
 *                 \boldsymbol{x}(0)}_{P}, &
 *     \underbrace{\boldsymbol{x}(1),\,\boldsymbol{x}(1),\,\ldots,\,
 *                 \boldsymbol{x}(1)}_{P}, &
 *     \ldots,
 *  \end{array}
 * @f]
 *
 * The following example decimates data in @c data.d while keeping their
 * original indices.
 *
 * @code{.sh}
 *   decimate -p 5 < data.d | interpolate -p 5 > data.dec
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int vector_length(kDefaultVectorLength);
  int start_index(kDefaultStartIndex);
  int interpolation_period(kDefaultInterpolationPeriod);
  OutputFormats output_format(kDefaultOutputFormat);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:s:p:o:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &vector_length) ||
            vector_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("interpolate", error_message);
          return 1;
        }
        break;
      }
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &vector_length) ||
            vector_length < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("interpolate", error_message);
          return 1;
        }
        ++vector_length;
        break;
      }
      case 's': {
        if (!sptk::ConvertStringToInteger(optarg, &start_index) ||
            start_index < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -s option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("interpolate", error_message);
          return 1;
        }
        break;
      }
      case 'p': {
        if (!sptk::ConvertStringToInteger(optarg, &interpolation_period) ||
            interpolation_period <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -p option must be a positive integer";
          sptk::PrintErrorMessage("interpolate", error_message);
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
          sptk::PrintErrorMessage("interpolate", error_message);
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
    sptk::PrintErrorMessage("interpolate", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("interpolate", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  const int output_length(interpolation_period * vector_length);
  std::vector<double> data(output_length);

  // Output zeros.
  for (int sample_index(0); sample_index < start_index; ++sample_index) {
    if (!sptk::WriteStream(0, vector_length, data, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write zero sequence";
      sptk::PrintErrorMessage("interpolate", error_message);
      return 1;
    }
  }

  while (sptk::ReadStream(false, 0, 0, vector_length, &data, &input_stream,
                          NULL)) {
    switch (output_format) {
      case kPadWithZero: {
        // nothing to do
        break;
      }
      case kPadWithSameValue: {
        for (int i(vector_length), j(0); i < output_length; ++i, ++j) {
          if (vector_length <= j) j = 0;
          data[i] = data[j];
        }
        break;
      }
      default: {
        break;
      }
    }

    if (!sptk::WriteStream(0, output_length, data, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write interpolated data sequence";
      sptk::PrintErrorMessage("interpolate", error_message);
      return 1;
    }
  }

  return 0;
}
