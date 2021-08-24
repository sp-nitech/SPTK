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

#include "SPTK/compression/huffman_encoding.h"

#include <algorithm>  // std::copy
#include <cstddef>    // std::size_t
#include <string>     // std::string
#include <utility>    // std::make_pair

namespace sptk {

HuffmanEncoding::HuffmanEncoding(std::ifstream* input_stream)
    : is_valid_(true) {
  if (NULL == input_stream) {
    is_valid_ = false;
    return;
  }

  {
    int symbol;
    std::string bits;
    while (*input_stream >> symbol >> bits) {
      std::vector<bool> codeword;
      for (char& bit : bits) {
        codeword.push_back('1' == bit ? true : false);
      }
      codebook_.insert(std::make_pair(symbol, codeword));
    }
  }

  if (codebook_.empty()) {
    is_valid_ = false;
    return;
  }
}

bool HuffmanEncoding::Run(int input, std::vector<bool>* output) const {
  // Check inputs.
  if (!is_valid_ || NULL == output) {
    return false;
  }

  if (codebook_.find(input) == codebook_.end()) {
    return false;
  }

  const std::size_t output_length(codebook_.at(input).size());
  if (output->size() != output_length) {
    output->resize(output_length);
  }

  std::copy(codebook_.at(input).begin(), codebook_.at(input).end(),
            output->begin());

  return true;
}

}  // namespace sptk
