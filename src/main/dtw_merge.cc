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
//                1996-2020  Nagoya Institute of Technology          //
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

#include <getopt.h>  // getopt_long

#include <fstream>   // std::ifstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream
#include <vector>    // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultNumOrder(25);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " dtw_merge - merge two vector sequences" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       dtw_merge [ options ] vfile file1 [ infile ] > stdout" << std::endl;  // NOLINT
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : length of vector   (   int)[" << std::setw(5) << std::right << kDefaultNumOrder + 1 << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : order of vector    (   int)[" << std::setw(5) << std::right << "l-1"                << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  vfile:" << std::endl;
  *stream << "       Viterbi path               (   int)" << std::endl;
  *stream << "  file1:" << std::endl;
  *stream << "       reference vector sequence  (double)" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       query vector sequence      (double)[stdin]" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       warped vector sequence     (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a dtw_merge [ @e option ] @e vfile @e file1 [ @e infile ]
 *
 * - @b -l @e int
 *   - length of vector @f$(1 \le M+1)@f$
 * - @b -m @e int
 *   - order of vector @f$(0 \le M)@f$
 * - @b vfile @e str
 *   - int-type Viterbi path
 * - @b file1 @e str
 *   - double-type reference vector sequence
 * - @b infile @e str
 *   - double-type query vector sequence
 * - @b stdout
 *   - double-type concatenated vector sequence
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("dtw_merge", error_message);
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
          sptk::PrintErrorMessage("dtw_merge", error_message);
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

  const char* viterbi_path_file;
  const char* reference_file;
  const char* query_file;
  const int num_input_files(argc - optind);
  if (3 == num_input_files) {
    viterbi_path_file = argv[argc - 3];
    reference_file = argv[argc - 2];
    query_file = argv[argc - 1];
  } else if (2 == num_input_files) {
    viterbi_path_file = argv[argc - 2];
    reference_file = argv[argc - 1];
    query_file = NULL;
  } else {
    std::ostringstream error_message;
    error_message << "Three input files are required";
    sptk::PrintErrorMessage("dtw_merge", error_message);
    return 1;
  }

  std::ifstream ifs1;
  ifs1.open(viterbi_path_file, std::ios::in | std::ios::binary);
  if (ifs1.fail()) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << viterbi_path_file;
    sptk::PrintErrorMessage("dtw_merge", error_message);
    return 1;
  }
  std::istream& input_stream_for_path(ifs1);

  std::ifstream ifs2;
  ifs2.open(reference_file, std::ios::in | std::ios::binary);
  if (ifs2.fail()) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << reference_file;
    sptk::PrintErrorMessage("dtw_merge", error_message);
    return 1;
  }
  std::istream& input_stream_for_reference(ifs2);

  std::ifstream ifs3;
  ifs3.open(query_file, std::ios::in | std::ios::binary);
  if (ifs3.fail() && NULL != query_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << query_file;
    sptk::PrintErrorMessage("dtw_merge", error_message);
    return 1;
  }
  std::istream& input_stream_for_query(ifs3.fail() ? std::cin : ifs3);

  const int length(num_order + 1);
  std::vector<double> query_vector(length);
  std::vector<double> reference_vector(length);
  if (!sptk::ReadStream(false, 0, 0, length, &query_vector,
                        &input_stream_for_query, NULL) ||
      !sptk::ReadStream(false, 0, 0, length, &reference_vector,
                        &input_stream_for_reference, NULL)) {
    return 0;
  }

  int prev_query_vector_index(0), prev_reference_vector_index(0);
  int curr_query_vector_index, curr_reference_vector_index;
  while (
      sptk::ReadStream(&curr_query_vector_index, &input_stream_for_path) &&
      sptk::ReadStream(&curr_reference_vector_index, &input_stream_for_path)) {
    if (curr_query_vector_index < 0 || curr_reference_vector_index < 0 ||
        curr_query_vector_index < prev_query_vector_index ||
        curr_reference_vector_index < prev_reference_vector_index) {
      std::ostringstream error_message;
      error_message << "Invalid Viterbi path";
      sptk::PrintErrorMessage("dtw_merge", error_message);
      return 1;
    }

    {
      const int diff(curr_query_vector_index - prev_query_vector_index);
      for (int i(0); i < diff; ++i) {
        if (!sptk::ReadStream(false, 0, 0, length, &query_vector,
                              &input_stream_for_query, NULL)) {
          return 0;
        }
      }
      prev_query_vector_index = curr_query_vector_index;
    }

    {
      const int diff(curr_reference_vector_index - prev_reference_vector_index);
      for (int i(0); i < diff; ++i) {
        if (!sptk::ReadStream(false, 0, 0, length, &reference_vector,
                              &input_stream_for_reference, NULL)) {
          return 0;
        }
      }
      prev_reference_vector_index = curr_reference_vector_index;
    }

    if (!sptk::WriteStream(0, length, query_vector, &std::cout, NULL) ||
        !sptk::WriteStream(0, length, reference_vector, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write merged vector";
      sptk::PrintErrorMessage("dtw_merge", error_message);
      return 1;
    }
  }

  return 0;
}
