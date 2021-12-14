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

const int kDefaultStartIndex(0);
const int kDefaultVectorLength(1);
const int kDefaultDecimationPeriod(10);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " decimate - data decimation" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       decimate [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : length of vector   (   int)[" << std::setw(5) << std::right << kDefaultVectorLength     << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : order of vector    (   int)[" << std::setw(5) << std::right << "l-1"                    << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -s s  : start index        (   int)[" << std::setw(5) << std::right << kDefaultStartIndex       << "][ 0 <= s <=   ]" << std::endl;  // NOLINT
  *stream << "       -p p  : decimation period  (   int)[" << std::setw(5) << std::right << kDefaultDecimationPeriod << "][ 1 <= p <=   ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence              (double)[stdin]" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       decimated data sequence    (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a decimate [ @e option ] [ @e infile ]
 *
 * - @b -l @e int
 *   - length of vector @f$(1 \le L)@f$
 * - @b -m @e int
 *   - order of vector @f$(0 \le M)@f$
 * - @b -s @e int
 *   - start index @f$(0 \le S)@f$
 * - @b -p @e int
 *   - decimation period @f$(1 \le P)@f$
 * - @b infile @e str
 *   - double-type data sequence
 * - @b stdout
 *   - double-type decimated data sequence
 *
 * The input is a sequence of @f$L@f$-dimensional vectors:
 * @f[
 *   \begin{array}{cccc}
 *     \boldsymbol{x}(0), & \boldsymbol{x}(1), & \boldsymbol{x}(2), & \ldots,
 *   \end{array}
 * @f]
 * where @f$L=M+1@f$. The output is the sequence resampled from the input:
 * @f[
 *   \begin{array}{cccc}
 *     \boldsymbol{x}(S), & \boldsymbol{x}(S+P), & \boldsymbol{x}(S+2P), &
 *     \ldots,
 *   \end{array}
 * @f]
 * where @f$S@f$ is the start index and @f$P@f$ is the decimation factor.
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
  int start_index(kDefaultStartIndex);
  int vector_length(kDefaultVectorLength);
  int decimation_period(kDefaultDecimationPeriod);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:s:p:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &vector_length) ||
            vector_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("decimate", error_message);
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
          sptk::PrintErrorMessage("decimate", error_message);
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
          sptk::PrintErrorMessage("decimate", error_message);
          return 1;
        }
        break;
      }
      case 'p': {
        if (!sptk::ConvertStringToInteger(optarg, &decimation_period) ||
            decimation_period <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -p option must be a positive integer";
          sptk::PrintErrorMessage("decimate", error_message);
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

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("decimate", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("decimate", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  std::vector<double> input_data(vector_length);

  // Skip data.
  for (int sample_index(0); sample_index < start_index; ++sample_index) {
    if (!sptk::ReadStream(false, 0, 0, vector_length, &input_data,
                          &input_stream, NULL)) {
      std::ostringstream error_message;
      error_message << "Start index exceeds data length";
      sptk::PrintErrorMessage("decimate", error_message);
      return 1;
    }
  }

  for (int sample_index(0); sptk::ReadStream(false, 0, 0, vector_length,
                                             &input_data, &input_stream, NULL);
       ++sample_index) {
    if (0 == sample_index % decimation_period) {
      if (!sptk::WriteStream(0, vector_length, input_data, &std::cout, NULL)) {
        std::ostringstream error_message;
        error_message << "Failed to write decimated data sequence";
        sptk::PrintErrorMessage("decimate", error_message);
        return 1;
      }
      sample_index = 0;
    }
  }

  return 0;
}
