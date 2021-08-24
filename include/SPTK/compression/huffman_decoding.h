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

#ifndef SPTK_COMPRESSION_HUFFMAN_DECODING_H_
#define SPTK_COMPRESSION_HUFFMAN_DECODING_H_

#include <fstream>  // std::ifstream

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Decode symbols from binary sequence.
 *
 * The input is a codeword and the output is the corresponding symbol.
 */
class HuffmanDecoding {
 public:
  /**
   * @param[in] input_stream Stream which contains codebook.
   */
  explicit HuffmanDecoding(std::ifstream* input_stream);

  virtual ~HuffmanDecoding() {
    Free(root_);
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] input A bit.
   * @param[in] output Symbol.
   * @param[in] is_leaf True if output is overwritten.
   */
  bool Get(bool input, int* output, bool* is_leaf);

 private:
  struct Node {
    Node* left;
    Node* right;
    int symbol;
  };

  void Free(Node* node) {
    if (NULL == node) return;
    Free(node->left);
    Free(node->right);
    delete node;
  }

  bool is_valid_;

  Node* root_;
  Node* curr_node_;

  DISALLOW_COPY_AND_ASSIGN(HuffmanDecoding);
};

}  // namespace sptk

#endif  // SPTK_COMPRESSION_HUFFMAN_DECODING_H_
