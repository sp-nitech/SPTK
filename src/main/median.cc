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
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

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
  const int num_vector(input_vectors.size());
  const int half_num_vector(num_vector / 2);
  const int vector_length(input_vectors[0].size());

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

  // get input file
  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("median", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  // open stream
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
