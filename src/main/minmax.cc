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
//                1996-2018  Nagoya Institute of Technology          //
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

#include "SPTK/math/minmax_accumulator.h"
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
  *stream << "       -f f  : way to find value           (   int)[" << std::setw(5) << std::right << kDefaultWayToFindValue << "][ 0 <= f <= 1 ]" << std::endl;  // NOLINT
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
  *stream << "       if f = 0, l must be greater than max(1, b - 1)" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

bool WriteMinMaxValues(
    const sptk::MinMaxAccumulator& minmax_accumulator,
    const std::vector<sptk::MinMaxAccumulator::Buffer>& buffer, int num_best,
    OutputFormats output_format, std::ostream* stream_for_position) {
  const int vector_length(buffer.size());

  if (kMinimumAndMaximum == output_format || kMinimum == output_format) {
    for (int rank(1); rank <= num_best; ++rank) {
      for (int vector_index(0); vector_index < vector_length; ++vector_index) {
        int position;
        double value;
        if (!minmax_accumulator.GetMinimum(buffer[vector_index], rank,
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
        if (!minmax_accumulator.GetMaximum(buffer[vector_index], rank,
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

int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);
  int num_best(kDefaultNumBest);
  OutputFormats output_format(kDefaultOutputFormat);
  WaysToFindValue way_to_find_value(kDefaultWayToFindValue);
  const char* position_file(NULL);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:b:o:f:p:h", NULL, NULL));
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
      case 'f': {
        const int min(0);
        const int max(static_cast<int>(kNumWaysToFindValue) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -f option must be an integer "
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
      (0 == num_order || num_order < num_best - 1)) {
    std::ostringstream error_message;
    error_message << "Length of vector must be greater than max(1, b - 1)";
    sptk::PrintErrorMessage("minmax", error_message);
    return 1;
  }

  // get input file
  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("minmax", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  // open input stream
  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("minmax", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  // open output stream
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

  // prepare for finding values
  sptk::MinMaxAccumulator minmax_accumulator(num_best);
  std::vector<sptk::MinMaxAccumulator::Buffer> buffer(
      kFindValueFromVector == way_to_find_value ? 1 : num_order + 1);
  if (!minmax_accumulator.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to set condition for finding values";
    sptk::PrintErrorMessage("minmax", error_message);
    return 1;
  }

  const int vector_length(num_order + 1);
  std::vector<double> data(vector_length);

  if (kFindValueFromVector == way_to_find_value) {
    while (sptk::ReadStream(false, 0, 0, vector_length, &data, &input_stream,
                            NULL)) {
      for (int vector_index(0); vector_index < vector_length; ++vector_index) {
        if (!minmax_accumulator.Run(data[vector_index], &buffer[0])) {
          std::ostringstream error_message;
          error_message << "Failed to find values";
          sptk::PrintErrorMessage("minmax", error_message);
          return 1;
        }
      }
      if (!WriteMinMaxValues(minmax_accumulator, buffer, num_best,
                             output_format, output_stream_pointer)) {
        std::ostringstream error_message;
        error_message << "Failed to write values";
        sptk::PrintErrorMessage("minmax", error_message);
        return 1;
      }
      minmax_accumulator.Clear(&buffer[0]);
    }
  } else if (kFindValueFromVectorSequenceForEachDimension ==
             way_to_find_value) {
    while (sptk::ReadStream(false, 0, 0, vector_length, &data, &input_stream,
                            NULL)) {
      for (int vector_index(0); vector_index < vector_length; ++vector_index) {
        if (!minmax_accumulator.Run(data[vector_index],
                                    &buffer[vector_index])) {
          std::ostringstream error_message;
          error_message << "Failed to find values";
          sptk::PrintErrorMessage("minmax", error_message);
          return 1;
        }
      }
    }
    if (!WriteMinMaxValues(minmax_accumulator, buffer, num_best, output_format,
                           output_stream_pointer)) {
      std::ostringstream error_message;
      error_message << "Failed to write values";
      sptk::PrintErrorMessage("minmax", error_message);
      return 1;
    }
  }

  return 0;
}
