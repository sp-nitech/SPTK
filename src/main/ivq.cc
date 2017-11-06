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

#include "SPTK/quantizer/inverse_vector_quantization.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultNumOrder(25);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " ivq - decoder of vector quantization" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       ivq [ options ] cbfile [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : length of vector   (   int)[" << std::setw(5) << std::right << kDefaultNumOrder + 1 << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -n n  : order of vector    (   int)[" << std::setw(5) << std::right << "l-1"                << "][ 0 <= n <=   ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       codebook index             (   int)[stdin]" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       quantized vector           (double)" << std::endl;
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
          sptk::PrintErrorMessage("ivq", error_message);
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
          sptk::PrintErrorMessage("ivq", error_message);
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
  const char* codebook_index_file;
  const int num_rest_args(argc - optind);
  if (2 == num_rest_args) {
    codebook_vectors_file = argv[argc - 2];
    codebook_index_file = argv[argc - 1];
  } else if (1 == num_rest_args) {
    codebook_vectors_file = argv[argc - 1];
    codebook_index_file = NULL;
  } else {
    std::ostringstream error_message;
    error_message << "Just two input files, cbfile and infile, are required";
    sptk::PrintErrorMessage("ivq", error_message);
    return 1;
  }

  std::ifstream ifs1;
  ifs1.open(codebook_vectors_file, std::ios::in | std::ios::binary);
  if (ifs1.fail()) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << codebook_vectors_file;
    sptk::PrintErrorMessage("ivq", error_message);
    return 1;
  }
  std::istream& stream_for_codebook_vectors(ifs1);

  std::ifstream ifs2;
  ifs2.open(codebook_index_file, std::ios::in | std::ios::binary);
  if (ifs2.fail() && NULL != codebook_index_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << codebook_index_file;
    sptk::PrintErrorMessage("ivq", error_message);
    return 1;
  }
  std::istream& stream_for_codebook_index(ifs2.fail() ? std::cin : ifs2);

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

  sptk::InverseVectorQuantization inverse_vector_quantization(num_order);
  if (!inverse_vector_quantization.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to set condition for decoding";
    sptk::PrintErrorMessage("ivq", error_message);
    return 1;
  }

  int codebook_index;
  std::vector<double> quantized_vector(length);

  while (sptk::ReadStream(&codebook_index, &stream_for_codebook_index)) {
    if (!inverse_vector_quantization.Run(codebook_index, codebook_vectors,
                                         &quantized_vector)) {
      std::ostringstream error_message;
      error_message << "Failed to decode";
      sptk::PrintErrorMessage("ivq", error_message);
      return 1;
    }
    if (!sptk::WriteStream(0, length, quantized_vector, &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write quantized vector";
      sptk::PrintErrorMessage("ivq", error_message);
      return 1;
    }
  }

  return 0;
}
