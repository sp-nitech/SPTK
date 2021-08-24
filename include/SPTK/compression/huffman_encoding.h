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

#ifndef SPTK_COMPRESSION_HUFFMAN_ENCODING_H_
#define SPTK_COMPRESSION_HUFFMAN_ENCODING_H_

#include <fstream>        // std::ifstream
#include <unordered_map>  // std::unordered_map
#include <vector>         // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Encode symbols to binary sequence.
 *
 * The input is a symbol and the output is the corresponding codeword.
 */
class HuffmanEncoding {
 public:
  /**
   * @param[in] input_stream Stream which contains codebook.
   */
  explicit HuffmanEncoding(std::ifstream* input_stream);

  virtual ~HuffmanEncoding() {
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] input Symbol.
   * @param[out] output Codeword.
   * @return True on success, false on failure.
   */
  bool Run(int input, std::vector<bool>* output) const;

 private:
  std::unordered_map<int, std::vector<bool> > codebook_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(HuffmanEncoding);
};

}  // namespace sptk

#endif  // SPTK_COMPRESSION_HUFFMAN_ENCODING_H_
