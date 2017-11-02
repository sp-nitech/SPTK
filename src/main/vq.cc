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
//                1996-2017  Nagoya Institute of Technology          //
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

#include "SPTK/quantizer/vector_quantization.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultNumOrder(25);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " vq - vector quantzation" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       vq [ options ] cbfile [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : length of vector   (   int)[" << std::setw(5) << std::right << kDefaultNumOrder + 1 << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -n n  : order of vector    (   int)[" << std::setw(5) << std::right << "l-1"                << "][ 0 <= n <=   ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       vector                     (double)[stdin]" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       codebook index             (   int)" << std::endl;
  *stream << "  cbfile:" << std::endl;
  *stream << "       codebook vectors           (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:n:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("vq", error_message);
          return 1;
        }
        --num_order;
        break;
      }
      case 'n': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -n option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("vq", error_message);
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

  // Get input file names.
  const char* codebook_vectors_file;
  const char* input_vectors_file;
  const int num_rest_args(argc - optind);
  if (2 == num_rest_args) {
    codebook_vectors_file = argv[argc - 2];
    input_vectors_file = argv[argc - 1];
  } else if (1 == num_rest_args) {
    codebook_vectors_file = argv[argc - 1];
    input_vectors_file = NULL;
  } else {
    std::ostringstream error_message;
    error_message << "Just two input files, cbfile and infile, are required";
    sptk::PrintErrorMessage("vq", error_message);
    return 1;
  }

  std::ifstream ifs1;
  ifs1.open(codebook_vectors_file, std::ios::in | std::ios::binary);
  if (ifs1.fail()) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << codebook_vectors_file;
    sptk::PrintErrorMessage("vq", error_message);
    return 1;
  }
  std::istream& stream_for_codebook_vectors(ifs1);

  std::ifstream ifs2;
  ifs2.open(input_vectors_file, std::ios::in | std::ios::binary);
  if (ifs2.fail() && NULL != input_vectors_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_vectors_file;
    sptk::PrintErrorMessage("vq", error_message);
    return 1;
  }
  std::istream& stream_for_input_vectors(ifs2.fail() ? std::cin : ifs2);

  // read codebook
  const int length(num_order + 1);
  std::vector<std::vector<double> > codebook_vectors;
  {
    std::vector<double> codebook_vector(length);
    while (sptk::ReadStream(false, 0, 0, length, &codebook_vector,
                            &stream_for_codebook_vectors)) {
      codebook_vectors.push_back(codebook_vector);
    }
  }

  sptk::VectorQuantization vector_quantization(num_order);
  if (!vector_quantization.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to set condition for quantization";
    sptk::PrintErrorMessage("vq", error_message);
    return 1;
  }

  std::vector<double> input_vector(length);

  while (sptk::ReadStream(false, 0, 0, length, &input_vector,
                          &stream_for_input_vectors)) {
    int codebook_index;
    if (!vector_quantization.Run(input_vector, codebook_vectors,
                                 &codebook_index)) {
      std::ostringstream error_message;
      error_message << "Failed to quantize";
      sptk::PrintErrorMessage("vq", error_message);
      return 1;
    }
    if (!sptk::WriteStream(codebook_index, &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write codebook index";
      sptk::PrintErrorMessage("vq", error_message);
      return 1;
    }
  }

  return 0;
}
