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
