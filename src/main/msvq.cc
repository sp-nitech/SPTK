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

#include "Getopt/getoptwin.h"
#include "SPTK/compression/multistage_vector_quantization.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultNumOrder(25);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " msvq - multistage vector quantization" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       msvq [ options ] -s cbfile1 -s cbfile2 ... [ infile ] > stdout" << std::endl;  // NOLINT
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : length of vector   (   int)[" << std::setw(5) << std::right << kDefaultNumOrder + 1 << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : order of vector    (   int)[" << std::setw(5) << std::right << "l-1"                << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -s s  : codebook file      (string)[" << std::setw(5) << std::right << "N/A"                << "]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  cbfile:" << std::endl;
  *stream << "       codebook                   (double)" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       vector                     (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       codebook index             (   int)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a msvq [ @e option ] [ @e infile ]
 *
 * - @b -l @e int
 *   - length of vector @f$(1 \le M + 1)@f$
 * - @b -m @e int
 *   - order of vector @f$(0 \le M)@f$
 * - @b -s @e str
 *   - codebook file
 * - @b infile @e str
 *   - double-type vector to be quantized
 * - @b stdout
 *   - int-type codebook index
 *
 * The below example quantizes and reconstructs vectors in @c data.d.
 *
 * @code{.sh}
 *   msvq -s cbfile < data.d | imsvq -s cbfile > data.q
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
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
          sptk::PrintErrorMessage("msvq", error_message);
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
          sptk::PrintErrorMessage("msvq", error_message);
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

  const int num_stage(static_cast<int>(codebook_vectors_file.size()));
  if (0 == num_stage) {
    std::ostringstream error_message;
    error_message << "One or more -s options are required";
    sptk::PrintErrorMessage("msvq", error_message);
    return 1;
  }

  const int length(num_order + 1);
  std::vector<std::vector<std::vector<double> > > codebook_vectors;
  for (int n(0); n < num_stage; ++n) {
    std::vector<std::vector<double> > codebook;
    {
      std::ifstream ifs;
      ifs.open(codebook_vectors_file[n], std::ios::in | std::ios::binary);
      if (ifs.fail()) {
        std::ostringstream error_message;
        error_message << "Cannot open file " << codebook_vectors_file[n];
        sptk::PrintErrorMessage("msvq", error_message);
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

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("msvq", error_message);
    return 1;
  }
  const char* input_vectors_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_vectors_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_vectors_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_vectors_file;
    sptk::PrintErrorMessage("msvq", error_message);
    return 1;
  }
  std::istream& stream_for_input_vectors(ifs.fail() ? std::cin : ifs);

  sptk::MultistageVectorQuantization multistage_vector_quantization(num_order,
                                                                    num_stage);
  sptk::MultistageVectorQuantization::Buffer buffer;
  if (!multistage_vector_quantization.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize MultistageVectorQuantization";
    sptk::PrintErrorMessage("msvq", error_message);
    return 1;
  }

  std::vector<double> input_vector(length);
  std::vector<int> codebook_indices(num_stage);

  while (sptk::ReadStream(false, 0, 0, length, &input_vector,
                          &stream_for_input_vectors, NULL)) {
    if (!multistage_vector_quantization.Run(input_vector, codebook_vectors,
                                            &codebook_indices, &buffer)) {
      std::ostringstream error_message;
      error_message << "Failed to quantize vector";
      sptk::PrintErrorMessage("msvq", error_message);
      return 1;
    }
    if (!sptk::WriteStream(0, num_stage, codebook_indices, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write codebook index";
      sptk::PrintErrorMessage("msvq", error_message);
      return 1;
    }
  }

  return 0;
}
