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

#include "GETOPT/ya_getopt.h"
#include "SPTK/math/product_accumulation.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kMagicNumberForEndOfFile(-1);
const int kDefaultVectorLength(1);
const bool kDefaultCumulativeModeFlag(false);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " vprod - product of vectors" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       vprod [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : length of vector   (   int)[" << std::setw(5) << std::right << kDefaultVectorLength << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : order of vector    (   int)[" << std::setw(5) << std::right << "l-1"                << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -t t  : output interval    (   int)[" << std::setw(5) << std::right << "EOF"                << "][ 1 <= t <=   ]" << std::endl;  // NOLINT
  *stream << "       -c    : cumulative mode    (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultCumulativeModeFlag) << "]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       vectors                    (double)[stdin]" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       product of vectors         (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a vprod [ @e option ] [ @e infile ]
 *
 * - @b -l @e int
 *   - length of vector @f$(1 \le L)@f$
 * - @b -m @e int
 *   - order of vector @f$(0 \le L - 1)@f$
 * - @b -t @e int
 *   - output interval @f$(1 \le T)@f$
 * - @b -c
 *   - cumulative mode
 * - @b infile @e str
 *   - double-type vectors
 * - @b stdout
 *   - double-type product
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
 *     \underbrace{p_{0}(1), \; \ldots, \; p_{0}(L)}_L, &
 *     \underbrace{p_{T}(1), \; \ldots, \; p_{T}(L)}_L, &
 *     \ldots,
 *   \end{array}
 * @f]
 * where
 * @f[
 *   p_t(l) = \prod_{\tau=1}^{T} x_{t+\tau}(l).
 * @f]
 * If @f$T@f$ is not given, the product of the whole input is computed.
 *
 * @code{.sh}
 *   echo 1 2 3 4 5 | x2x +ad | vprod | x2x +da
 *   # 120
 * @endcode
 *
 * @code{.sh}
 *   echo 2 3 4 5 | x2x +ad | vprod -c -t 1 | x2x +da
 *   # 2
 *   # 6
 *   # 24
 *   # 120
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int vector_length(kDefaultVectorLength);
  int output_interval(kMagicNumberForEndOfFile);
  bool cumulative_mode_flag(kDefaultCumulativeModeFlag);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:t:ch", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &vector_length) ||
            vector_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("vprod", error_message);
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
          sptk::PrintErrorMessage("vprod", error_message);
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
          sptk::PrintErrorMessage("vprod", error_message);
          return 1;
        }
        break;
      }
      case 'c': {
        cumulative_mode_flag = true;
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
    sptk::PrintErrorMessage("vprod", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  if (!sptk::SetBinaryMode()) {
    std::ostringstream error_message;
    error_message << "Cannot set translation mode";
    sptk::PrintErrorMessage("vprod", error_message);
    return 1;
  }

  std::ifstream ifs;
  if (NULL != input_file) {
    ifs.open(input_file, std::ios::in | std::ios::binary);
    if (ifs.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << input_file;
      sptk::PrintErrorMessage("vprod", error_message);
      return 1;
    }
  }
  std::istream& input_stream(ifs.is_open() ? ifs : std::cin);

  sptk::ProductAccumulation accumulation(vector_length - 1);
  sptk::ProductAccumulation::Buffer buffer;
  if (!accumulation.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize ProductAccumulation";
    sptk::PrintErrorMessage("vprod", error_message);
    return 1;
  }

  std::vector<double> data(vector_length);
  std::vector<double> product(vector_length);
  for (int vector_index(1);
       sptk::ReadStream(false, 0, 0, vector_length, &data, &input_stream, NULL);
       ++vector_index) {
    if (!accumulation.Run(data, &buffer)) {
      std::ostringstream error_message;
      error_message << "Failed to accumulate product";
      sptk::PrintErrorMessage("vprod", error_message);
      return 1;
    }

    if (kMagicNumberForEndOfFile != output_interval &&
        0 == vector_index % output_interval) {
      if (!accumulation.GetProduct(buffer, &product)) {
        std::ostringstream error_message;
        error_message << "Failed to accumulate product";
        sptk::PrintErrorMessage("vprod", error_message);
        return 1;
      }
      if (!sptk::WriteStream(0, vector_length, product, &std::cout, NULL)) {
        std::ostringstream error_message;
        error_message << "Failed to write product";
        sptk::PrintErrorMessage("vprod", error_message);
        return 1;
      }
      if (!cumulative_mode_flag) {
        accumulation.Clear(&buffer);
      }
    }
  }

  int num_data;
  if (!accumulation.GetNumData(buffer, &num_data)) {
    std::ostringstream error_message;
    error_message << "Failed to accumulate product";
    sptk::PrintErrorMessage("vprod", error_message);
    return 1;
  }

  if (kMagicNumberForEndOfFile == output_interval && 0 < num_data) {
    if (!accumulation.GetProduct(buffer, &product)) {
      std::ostringstream error_message;
      error_message << "Failed to compute product";
      sptk::PrintErrorMessage("vprod", error_message);
      return 1;
    }
    if (!sptk::WriteStream(0, vector_length, product, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write prodct";
      sptk::PrintErrorMessage("vprod", error_message);
      return 1;
    }
  }

  return 0;
}
