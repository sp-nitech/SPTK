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
