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

#include "Getopt/getoptwin.h"
#include "SPTK/math/minmax_accumulation.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum OutputFormats {
  kMinimumAndMaximum,
  kMinimum,
  kMaximum,
  kNumOutputFormats
};

enum WaysToFindValue {
  kFindValueFromVector,
  kFindValueFromVectorSequenceForEachDimension,
  kNumWaysToFindValue
};

const int kDefaultNumOrder(0);
const int kDefaultNumBest(1);
const OutputFormats kDefaultOutputFormat(kMinimumAndMaximum);
const WaysToFindValue kDefaultWayToFindValue(
    kFindValueFromVectorSequenceForEachDimension);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " minmax - find minimum and maximum values" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       minmax [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : length of vector            (   int)[" << std::setw(5) << std::right << kDefaultNumOrder + 1   << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : order of vector             (   int)[" << std::setw(5) << std::right << "l-1"                  << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -b b  : find N-best values          (   int)[" << std::setw(5) << std::right << kDefaultNumBest        << "][ 1 <= b <=   ]" << std::endl;  // NOLINT
  *stream << "       -o o  : output format               (   int)[" << std::setw(5) << std::right << kDefaultOutputFormat   << "][ 0 <= o <= 2 ]" << std::endl;  // NOLINT
  *stream << "                 0 (minimum and maximum)" << std::endl;
  *stream << "                 1 (minimum)" << std::endl;
  *stream << "                 2 (maximum)" << std::endl;
  *stream << "       -w w  : way to find value           (   int)[" << std::setw(5) << std::right << kDefaultWayToFindValue << "][ 0 <= w <= 1 ]" << std::endl;  // NOLINT
  *stream << "                 0 (find value from a vector)" << std::endl;
  *stream << "                 1 (find value from vector sequence for each dimension)" << std::endl;  // NOLINT
  *stream << "       -p p  : output filename of int type (string)[" << std::setw(5) << std::right << "N/A"                  << "]" << std::endl;  // NOLINT
  *stream << "               position of found value" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence                       (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       minimum and maximum values          (double)" << std::endl;
  *stream << "  notice:" << std::endl;
  *stream << "       if w = 0, l must be greater than max(1, b - 1)" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

bool WriteMinMaxValues(
    const sptk::MinMaxAccumulation& minmax_accumulation,
    const std::vector<sptk::MinMaxAccumulation::Buffer>& buffer, int num_best,
    OutputFormats output_format, std::ostream* stream_for_position) {
  const int vector_length(static_cast<int>(buffer.size()));

  if (kMinimumAndMaximum == output_format || kMinimum == output_format) {
    for (int rank(1); rank <= num_best; ++rank) {
      for (int vector_index(0); vector_index < vector_length; ++vector_index) {
        int position;
        double value;
        if (!minmax_accumulation.GetMinimum(buffer[vector_index], rank,
                                            &position, &value)) {
          return false;
        }
        if (NULL != stream_for_position &&
            !sptk::WriteStream(position, stream_for_position)) {
          return false;
        }
        if (!sptk::WriteStream(value, &std::cout)) {
          return false;
        }
      }
    }
  }

  if (kMinimumAndMaximum == output_format || kMaximum == output_format) {
    for (int rank(1); rank <= num_best; ++rank) {
      for (int vector_index(0); vector_index < vector_length; ++vector_index) {
        int position;
        double value;
        if (!minmax_accumulation.GetMaximum(buffer[vector_index], rank,
                                            &position, &value)) {
          return false;
        }
        if (NULL != stream_for_position &&
            !sptk::WriteStream(position, stream_for_position)) {
          return false;
        }
        if (!sptk::WriteStream(value, &std::cout)) {
          return false;
        }
      }
    }
  }

  return true;
}

}  // namespace

/**
 * @a minmax [ @e option ] [ @e infile ]
 *
 * - @b -l @e int
 *   - length of vector @f$(1 \le M + 1)@f$
 * - @b -m @e int
 *   - order of vector @f$(0 \le M)@f$
 * - @b -b @e int
 *   - find @f$N@f$-best values @f$(1 \le N)@f$
 * - @b -o @e int
 *   - output format
 *     \arg @c 0 minimum and maximum
 *     \arg @c 1 minimum
 *     \arg @c 2 maximum
 * - @b -w @e int
 *   - way to find value
 *     \arg @c 0 find value from a vector
 *     \arg @c 1 find value from vector sequence for each dimension
 * - @b -p @e str
 *   - int-type positions
 * - @b infile @e str
 *   - double-type data sequence
 * - @b stdout
 *   - double-type minimum and maximum values
 *
 * @code{.sh}
 *   ramp -l 10 | minmax -l 5 -o 1 -w 0 | x2x +da
 *   # 0, 5
 * @endcode
 *
 * @code{.sh}
 *   ramp -l 10 | minmax -l 5 -o 1 -w 1 | x2x +da
 *   # 0, 1, 2, 3, 4
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);
  int num_best(kDefaultNumBest);
  OutputFormats output_format(kDefaultOutputFormat);
  WaysToFindValue way_to_find_value(kDefaultWayToFindValue);
  const char* position_file(NULL);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:b:o:w:p:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("minmax", error_message);
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
          sptk::PrintErrorMessage("minmax", error_message);
          return 1;
        }
        break;
      }
      case 'b': {
        if (!sptk::ConvertStringToInteger(optarg, &num_best) || num_best <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -b option must be a positive integer";
          sptk::PrintErrorMessage("minmax", error_message);
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
          sptk::PrintErrorMessage("minmax", error_message);
          return 1;
        }
        output_format = static_cast<OutputFormats>(tmp);
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
          sptk::PrintErrorMessage("minmax", error_message);
          return 1;
        }
        way_to_find_value = static_cast<WaysToFindValue>(tmp);
        break;
      }
      case 'p': {
        position_file = optarg;
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
    sptk::PrintErrorMessage("minmax", error_message);
    return 1;
  }

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("minmax", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("minmax", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  std::ofstream ofs;
  if (NULL != position_file) {
    ofs.open(position_file, std::ios::out | std::ios::binary);
    if (ofs.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << position_file;
      sptk::PrintErrorMessage("minmax", error_message);
      return 1;
    }
  }
  std::ostream& output_stream(ofs);
  std::ostream* output_stream_pointer(NULL == position_file ? NULL
                                                            : &output_stream);

  sptk::MinMaxAccumulation minmax_accumulation(num_best);
  std::vector<sptk::MinMaxAccumulation::Buffer> buffer(
      kFindValueFromVector == way_to_find_value ? 1 : num_order + 1);
  if (!minmax_accumulation.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize MinMaxAccumulation";
    sptk::PrintErrorMessage("minmax", error_message);
    return 1;
  }

  const int vector_length(num_order + 1);
  std::vector<double> data(vector_length);

  if (kFindValueFromVector == way_to_find_value) {
    while (sptk::ReadStream(false, 0, 0, vector_length, &data, &input_stream,
                            NULL)) {
      for (int vector_index(0); vector_index < vector_length; ++vector_index) {
        if (!minmax_accumulation.Run(data[vector_index], &buffer[0])) {
          std::ostringstream error_message;
          error_message << "Failed to find values";
          sptk::PrintErrorMessage("minmax", error_message);
          return 1;
        }
      }
      if (!WriteMinMaxValues(minmax_accumulation, buffer, num_best,
                             output_format, output_stream_pointer)) {
        std::ostringstream error_message;
        error_message << "Failed to write values";
        sptk::PrintErrorMessage("minmax", error_message);
        return 1;
      }
      minmax_accumulation.Clear(&buffer[0]);
    }
  } else if (kFindValueFromVectorSequenceForEachDimension ==
             way_to_find_value) {
    while (sptk::ReadStream(false, 0, 0, vector_length, &data, &input_stream,
                            NULL)) {
      for (int vector_index(0); vector_index < vector_length; ++vector_index) {
        if (!minmax_accumulation.Run(data[vector_index],
                                     &buffer[vector_index])) {
          std::ostringstream error_message;
          error_message << "Failed to find values";
          sptk::PrintErrorMessage("minmax", error_message);
          return 1;
        }
      }
    }
    if (!WriteMinMaxValues(minmax_accumulation, buffer, num_best, output_format,
                           output_stream_pointer)) {
      std::ostringstream error_message;
      error_message << "Failed to write values";
      sptk::PrintErrorMessage("minmax", error_message);
      return 1;
    }
  }

  return 0;
}
