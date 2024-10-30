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

#include "GETOPT/ya_getopt.h"
#include "SPTK/math/mode_accumulation.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum WaysToFindValue {
  kFindValueFromVector,
  kFindValueFromVectorSequenceForEachDimension,
  kNumWaysToFindValue
};

const int kDefaultNumOrder(0);
const int kDefaultNumBest(1);
const WaysToFindValue kDefaultWayToFindValue(
    kFindValueFromVectorSequenceForEachDimension);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " mode - find mode value" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       mode [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : length of vector            (   int)[" << std::setw(5) << std::right << kDefaultNumOrder + 1   << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : order of vector             (   int)[" << std::setw(5) << std::right << "l-1"                  << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -b b  : find N-best values          (   int)[" << std::setw(5) << std::right << kDefaultNumBest        << "][ 1 <= b <=   ]" << std::endl;  // NOLINT
  *stream << "       -w w  : way to find value           (   int)[" << std::setw(5) << std::right << kDefaultWayToFindValue << "][ 0 <= w <= 1 ]" << std::endl;  // NOLINT
  *stream << "                 0 (find value from a vector)" << std::endl;
  *stream << "                 1 (find value from vector sequence for each dimension)" << std::endl;  // NOLINT
  *stream << "       -c c  : output filename of int type (string)[" << std::setw(5) << std::right << "N/A"                  << "]" << std::endl;  // NOLINT
  *stream << "               count of found value" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence                       (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       mode                                (double)" << std::endl;
  *stream << "  notice:" << std::endl;
  *stream << "       if w = 0, l must be greater than max(1, b - 1)" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

bool WriteModeValues(const sptk::ModeAccumulation& mode_accumulation,
                     const std::vector<sptk::ModeAccumulation::Buffer>& buffer,
                     int num_best, std::ostream* stream_for_count) {
  const int vector_length(static_cast<int>(buffer.size()));

  for (int rank(1); rank <= num_best; ++rank) {
    for (int vector_index(0); vector_index < vector_length; ++vector_index) {
      double value;
      int count;
      if (!mode_accumulation.GetMode(rank, buffer[vector_index], &value,
                                     &count)) {
        return false;
      }
      if (NULL != stream_for_count &&
          !sptk::WriteStream(count, stream_for_count)) {
        return false;
      }
      if (!sptk::WriteStream(value, &std::cout)) {
        return false;
      }
    }
  }

  return true;
}

}  // namespace

/**
 * @a mode [ @e option ] [ @e infile ]
 *
 * - @b -l @e int
 *   - length of vector @f$(1 \le M + 1)@f$
 * - @b -m @e int
 *   - order of vector @f$(0 \le M)@f$
 * - @b -b @e int
 *   - find @f$N@f$-best values @f$(1 \le N)@f$
 * - @b -w @e int
 *   - way to find value
 *     \arg @c 0 find value from a vector
 *     \arg @c 1 find value from vector sequence for each dimension
 * - @b -c @e str
 *   - int-type counts
 * - @b infile @e str
 *   - double-type data sequence
 * - @b stdout
 *   - double-type mode
 *
 * @code{.sh}
 *   echo 0 3 3 4 8 8 8 1 | x2x +ad | mode -b 2 -w 1 | x2x +da
 *   # 8, 3
 * @endcode
 *
 * @code{.sh}
 *   echo 0 3 3 4 8 8 8 1 | x2x +ad | mode -l 4 -w 0 | x2x +da
 *   # 3, 8
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);
  int num_best(kDefaultNumBest);
  WaysToFindValue way_to_find_value(kDefaultWayToFindValue);
  const char* count_file(NULL);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:b:w:c:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("mode", error_message);
          return 1;
        }
        --num_order;
        break;
      }
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("mode", error_message);
          return 1;
        }
        break;
      }
      case 'b': {
        if (!sptk::ConvertStringToInteger(optarg, &num_best) || num_best <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -b option must be a positive integer";
          sptk::PrintErrorMessage("mode", error_message);
          return 1;
        }
        break;
      }
      case 'w': {
        const int min(0);
        const int max(static_cast<int>(kNumWaysToFindValue) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -w option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("mode", error_message);
          return 1;
        }
        way_to_find_value = static_cast<WaysToFindValue>(tmp);
        break;
      }
      case 'c': {
        count_file = optarg;
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

  if (kFindValueFromVector == way_to_find_value &&
      (0 == num_order || num_order + 1 < num_best)) {
    std::ostringstream error_message;
    error_message << "Length of vector must be greater than max(1, b - 1)";
    sptk::PrintErrorMessage("mode", error_message);
    return 1;
  }

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("mode", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  if (!sptk::SetBinaryMode()) {
    std::ostringstream error_message;
    error_message << "Cannot set translation mode";
    sptk::PrintErrorMessage("mode", error_message);
    return 1;
  }

  std::ifstream ifs;
  if (NULL != input_file) {
    ifs.open(input_file, std::ios::in | std::ios::binary);
    if (ifs.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << input_file;
      sptk::PrintErrorMessage("mode", error_message);
      return 1;
    }
  }
  std::istream& input_stream(ifs.is_open() ? ifs : std::cin);

  std::ofstream ofs;
  if (NULL != count_file) {
    ofs.open(count_file, std::ios::out | std::ios::binary);
    if (ofs.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << count_file;
      sptk::PrintErrorMessage("mode", error_message);
      return 1;
    }
  }
  std::ostream* output_stream_pointer(NULL == count_file ? NULL : &ofs);

  sptk::ModeAccumulation mode_accumulation(num_best);
  std::vector<sptk::ModeAccumulation::Buffer> buffer(
      kFindValueFromVector == way_to_find_value ? 1 : num_order + 1);
  if (!mode_accumulation.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize ModeAccumulation";
    sptk::PrintErrorMessage("mode", error_message);
    return 1;
  }

  const int vector_length(num_order + 1);
  std::vector<double> data(vector_length);

  if (kFindValueFromVector == way_to_find_value) {
    while (sptk::ReadStream(false, 0, 0, vector_length, &data, &input_stream,
                            NULL)) {
      for (int vector_index(0); vector_index < vector_length; ++vector_index) {
        if (!mode_accumulation.Run(data[vector_index], &buffer[0])) {
          std::ostringstream error_message;
          error_message << "Failed to find values";
          sptk::PrintErrorMessage("mode", error_message);
          return 1;
        }
      }
      if (!WriteModeValues(mode_accumulation, buffer, num_best,
                           output_stream_pointer)) {
        std::ostringstream error_message;
        error_message << "Failed to write values";
        sptk::PrintErrorMessage("mode", error_message);
        return 1;
      }
      mode_accumulation.Clear(&buffer[0]);
    }
  } else if (kFindValueFromVectorSequenceForEachDimension ==
             way_to_find_value) {
    bool empty(true);
    while (sptk::ReadStream(false, 0, 0, vector_length, &data, &input_stream,
                            NULL)) {
      for (int vector_index(0); vector_index < vector_length; ++vector_index) {
        if (!mode_accumulation.Run(data[vector_index], &buffer[vector_index])) {
          std::ostringstream error_message;
          error_message << "Failed to find values";
          sptk::PrintErrorMessage("mode", error_message);
          return 1;
        }
      }
      empty = false;
    }
    // Write value if at least one data is given.
    if (!empty) {
      if (!WriteModeValues(mode_accumulation, buffer, num_best,
                           output_stream_pointer)) {
        std::ostringstream error_message;
        error_message << "Failed to write values";
        sptk::PrintErrorMessage("mode", error_message);
        return 1;
      }
    }
  }

  return 0;
}
