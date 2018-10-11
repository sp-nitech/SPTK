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

#include <getopt.h>  // getopt_long
#include <fstream>   // std::ifstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream
#include <vector>    // std::vector

#include "SPTK/quantizer/inverse_multistage_vector_quantization.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultNumOrder(25);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " imsvq - decoder of multistage vector quantization" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       imsvq [ options ] -s cbfile1 -s cbfile2 ... [ infile ] > stdout" << std::endl;  // NOLINT
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : length of vector   (   int)[" << std::setw(5) << std::right << kDefaultNumOrder + 1 << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : order of vector    (   int)[" << std::setw(5) << std::right << "l-1"                << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -s s  : codebook file      (string)[" << std::setw(5) << std::right << "N/A"                << "]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  cbfile:" << std::endl;
  *stream << "       codebook                   (double)" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       codebook index             (   int)[stdin]" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       reconstructed vector       (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);
  std::vector<char*> codebook_vectors_file;

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:s:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("imsvq", error_message);
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
          sptk::PrintErrorMessage("imsvq", error_message);
          return 1;
        }
        break;
      }
      case 's': {
        codebook_vectors_file.push_back(optarg);
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

  const int num_stage(codebook_vectors_file.size());
  if (0 == num_stage) {
    std::ostringstream error_message;
    error_message << "One or more -s options are required";
    sptk::PrintErrorMessage("imsvq", error_message);
    return 1;
  }

  // read codebook
  const int length(num_order + 1);
  std::vector<std::vector<std::vector<double> > > codebook_vectors;
  for (int i(0); i < num_stage; ++i) {
    std::vector<std::vector<double> > codebook;
    {
      std::ifstream ifs;
      ifs.open(codebook_vectors_file[i], std::ios::in | std::ios::binary);
      if (ifs.fail()) {
        std::ostringstream error_message;
        error_message << "Cannot open file " << codebook_vectors_file[i];
        sptk::PrintErrorMessage("imsvq", error_message);
        return 1;
      }

      std::vector<double> tmp(length);
      while (sptk::ReadStream(false, 0, 0, length, &tmp, &ifs, NULL)) {
        codebook.push_back(tmp);
      }
      ifs.close();
    }
    codebook_vectors.push_back(codebook);
  }

  // get input file
  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("imsvq", error_message);
    return 1;
  }
  const char* codebook_index_file(0 == num_input_files ? NULL : argv[optind]);

  // open stream
  std::ifstream ifs;
  ifs.open(codebook_index_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != codebook_index_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << codebook_index_file;
    sptk::PrintErrorMessage("imsvq", error_message);
    return 1;
  }
  std::istream& stream_for_codebook_index(ifs.fail() ? std::cin : ifs);

  sptk::InverseMultistageVectorQuantization
      inverse_multistage_vector_quantization(num_order, num_stage);
  sptk::InverseMultistageVectorQuantization::Buffer buffer;
  if (!inverse_multistage_vector_quantization.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to set condition for decoding";
    sptk::PrintErrorMessage("imsvq", error_message);
    return 1;
  }

  std::vector<int> codebook_index(num_stage);
  std::vector<double> reconstructed_vector(length);
  while (sptk::ReadStream(false, 0, 0, num_stage, &codebook_index,
                          &stream_for_codebook_index, NULL)) {
    if (!inverse_multistage_vector_quantization.Run(
            codebook_index, codebook_vectors, &reconstructed_vector, &buffer)) {
      std::ostringstream error_message;
      error_message << "Failed to decode";
      sptk::PrintErrorMessage("imsvq", error_message);
      return 1;
    }
    if (!sptk::WriteStream(0, length, reconstructed_vector, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write reconstructed vector";
      sptk::PrintErrorMessage("imsvq", error_message);
      return 1;
    }
  }

  return 0;
}
