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
#include <string>    // std::string
#include <vector>    // std::vector

#include "Getopt/getoptwin.h"
#include "SPTK/compression/huffman_coding.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultStartIndex(0);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " huffman - Huffman coding" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       huffman [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -s s  : start index               (   int)[" << std::setw(5) << std::right << kDefaultStartIndex << "][   <= s <=   ]" << std::endl;  // NOLINT
  *stream << "       -L L  : output filename of double (string)[" << std::setw(5) << std::right << "N/A"              << "]" << std::endl;  // NOLINT
  *stream << "               type average code length" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       probability sequence              (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       codebook                          (string)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a huffman [ @e option ] [ @e infile ]
 *
 * - @b -s @e int
 *   - start index @f$(S)@f$
 * - @b -L @e str
 *   - output filename of double-type average code length
 * - @b infile @e str
 *   - double-type probability sequence
 * - @b stdout
 *   - ascii-type codebook
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int start_index(kDefaultStartIndex);
  const char* average_code_length_file(NULL);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "s:L:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 's': {
        if (!sptk::ConvertStringToInteger(optarg, &start_index)) {
          std::ostringstream error_message;
          error_message << "The argument for the -s option must be an integer";
          sptk::PrintErrorMessage("huffman", error_message);
          return 1;
        }
        break;
      }
      case 'L': {
        average_code_length_file = optarg;
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
    sptk::PrintErrorMessage("huffman", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("huffman", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  std::ofstream ofs;
  if (NULL != average_code_length_file) {
    ofs.open(average_code_length_file, std::ios::out | std::ios::binary);
    if (ofs.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << average_code_length_file;
      sptk::PrintErrorMessage("huffman", error_message);
      return 1;
    }
  }
  std::ostream& output_stream(ofs);

  std::vector<double> probabilities;
  {
    double tmp;
    while (sptk::ReadStream(&tmp, &input_stream)) {
      probabilities.push_back(tmp);
    }
  }
  if (probabilities.empty()) return 0;

  const int num_element(static_cast<int>(probabilities.size()));
  sptk::HuffmanCoding huffman_coding(num_element);
  if (!huffman_coding.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize HuffmanCoding";
    sptk::PrintErrorMessage("huffman", error_message);
    return 1;
  }

  std::vector<std::string> codewords(num_element);
  if (!huffman_coding.Run(probabilities, &codewords)) {
    std::ostringstream error_message;
    error_message << "Failed to perform Huffman coding";
    sptk::PrintErrorMessage("huffman", error_message);
    return 1;
  }

  // Output codebook.
  {
    std::vector<std::string> codebook(num_element);
    for (int i(0), index(start_index); i < num_element; ++i, ++index) {
      std::ostringstream oss;
      oss << index << " " << codewords[i];
      codebook[i] = oss.str();
    }
    if (!sptk::WriteStream(0, num_element, codebook, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write codebook";
      sptk::PrintErrorMessage("huffman", error_message);
      return 1;
    }
  }

  // Output average code length.
  if (NULL != average_code_length_file) {
    double average_code_length(0.0);
    for (int i(0); i < num_element; ++i) {
      average_code_length += probabilities[i] * codewords[i].length();
    }
    if (!sptk::WriteStream(average_code_length, &output_stream)) {
      std::ostringstream error_message;
      error_message << "Failed to write average code length";
      sptk::PrintErrorMessage("huffman", error_message);
      return 1;
    }
  }

  return 0;
}
