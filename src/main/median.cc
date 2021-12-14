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

#include <algorithm>  // std::sort
#include <fstream>    // std::ifstream
#include <iomanip>    // std::setw
#include <iostream>   // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>    // std::ostringstream
#include <vector>     // std::vector

#include "Getopt/getoptwin.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultVectorLength(1);
const int kMagicNumberForEndOfFile(-1);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " median - median of vectors" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       median [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : length of vector   (   int)[" << std::setw(5) << std::right << kDefaultVectorLength << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : order of vector    (   int)[" << std::setw(5) << std::right << "l-1"                << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -t t  : output interval    (   int)[" << std::setw(5) << std::right << "EOF"                << "][ 1 <= t <=   ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       vectors                    (double)[stdin]" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       median                     (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

bool OutputMedian(const std::vector<std::vector<double> >& input_vectors) {
  const int num_vector(static_cast<int>(input_vectors.size()));
  const int half_num_vector(num_vector / 2);
  const int vector_length(static_cast<int>(input_vectors[0].size()));

  std::vector<double> vector_for_sort(num_vector);
  for (int data_index(0); data_index < vector_length; ++data_index) {
    for (int i(0); i < num_vector; ++i) {
      vector_for_sort[i] = input_vectors[i][data_index];
    }
    std::sort(vector_for_sort.begin(), vector_for_sort.end());
    const double median(0 == num_vector % 2
                            ? (vector_for_sort[half_num_vector - 1] +
                               vector_for_sort[half_num_vector]) *
                                  0.5
                            : vector_for_sort[half_num_vector]);
    if (!sptk::WriteStream(median, &std::cout)) {
      return false;
    }
  }

  return true;
}

}  // namespace

/**
 * @a median [ @e option ] [ @e infile ]
 *
 * - @b -l @e int
 *   - length of vector @f$(1 \le L)@f$
 * - @b -m @e int
 *   - order of vector @f$(0 \le L - 1)@f$
 * - @b -t @e int
 *   - output interval @f$(1 \le T)@f$
 * - @b infile @e str
 *   - double-type vectors
 * - @b stdout
 *   - double-type median
 *
 * The input of this command is
 * @f[
 *   \begin{array}{ccc}
 *     \underbrace{x_1(1), \; \ldots, \; x_1(L)}_L, &
 *     \underbrace{x_2(1), \; \ldots, \; x_2(L)}_L, &
 *     \ldots,
 *   \end{array}
 * @f]
 * and the output is
 * @f[
 *   \begin{array}{ccc}
 *     \underbrace{m_{0}(1), \; \ldots, \; m_{0}(L)}_L, &
 *     \underbrace{m_{T}(1), \; \ldots, \; m_{T}(L)}_L, &
 *     \ldots,
 *   \end{array}
 * @f]
 * where @f$m_t(l)@f$ is the median value of
 * @f$\left\{ x_{t+\tau}(l) \right\}_{\tau=1}^T@f$.
 * If @f$T@f$ is not given, the median of the whole input is computed.
 *
 * @code{.sh}
 *   # The number of input is even:
 *   echo 0 1 2 3 4 5 | x2x +ad | median | x2x +da
 *   # 2.5
 * @endcode
 *
 * @code{.sh}
 *   # The number of input is odd:
 *   echo 0 1 2 3 4 5 | x2x +ad | median | x2x +da
 *   # 3
 * @endcode
 *
 * @code{.sh}
 *   echo 0 1 2 3 4 5 | x2x +ad | median -t 3 | x2x +da
 *   # 1
 *   # 4
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int vector_length(kDefaultVectorLength);
  int output_interval(kMagicNumberForEndOfFile);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:t:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &vector_length) ||
            vector_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("median", error_message);
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
          sptk::PrintErrorMessage("median", error_message);
          return 1;
        }
        ++vector_length;
        break;
      }
      case 't': {
        if (!sptk::ConvertStringToInteger(optarg, &output_interval) ||
            output_interval <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -t option must be a positive integer";
          sptk::PrintErrorMessage("median", error_message);
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
    sptk::PrintErrorMessage("median", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("median", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  std::vector<std::vector<double> > input_vectors;
  if (kMagicNumberForEndOfFile != output_interval) {
    input_vectors.reserve(output_interval);
  }

  std::vector<double> data(vector_length);
  for (int index(1);
       sptk::ReadStream(false, 0, 0, vector_length, &data, &input_stream, NULL);
       ++index) {
    input_vectors.push_back(data);
    if (kMagicNumberForEndOfFile != output_interval &&
        0 == index % output_interval) {
      if (!OutputMedian(input_vectors)) {
        std::ostringstream error_message;
        error_message << "Failed to write median";
        sptk::PrintErrorMessage("median", error_message);
        return 1;
      }
      input_vectors.clear();
    }
  }

  if (kMagicNumberForEndOfFile == output_interval && 0 < input_vectors.size()) {
    if (!OutputMedian(input_vectors)) {
      std::ostringstream error_message;
      error_message << "Failed to write median";
      sptk::PrintErrorMessage("median", error_message);
      return 1;
    }
  }

  return 0;
}
