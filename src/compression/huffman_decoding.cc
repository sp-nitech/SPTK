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

#include "SPTK/compression/huffman_decoding.h"

#include <string>  // std::string

namespace sptk {

HuffmanDecoding::HuffmanDecoding(std::ifstream* input_stream)
    : is_valid_(true), root_(NULL), curr_node_(NULL) {
  if (NULL == input_stream) {
    is_valid_ = false;
    return;
  }

  try {
    root_ = new Node;
    root_->left = NULL;
    root_->right = NULL;

    int symbol;
    std::string bits;
    while (*input_stream >> symbol >> bits) {
      Node* node(root_);
      for (char& bit : bits) {
        const bool right('1' == bit ? true : false);
        Node* next_node(right ? node->right : node->left);
        if (NULL == next_node) {
          next_node = new Node;
          next_node->left = NULL;
          next_node->right = NULL;
          if (right) {
            node->right = next_node;
          } else {
            node->left = next_node;
          }
        }
        node = next_node;
      }
      node->symbol = symbol;
    }
  } catch (...) {
    Free(root_);
    is_valid_ = false;
    return;
  }

  curr_node_ = root_;
}

bool HuffmanDecoding::Get(bool input, int* output, bool* is_leaf) {
  // Check inputs.
  if (!is_valid_ || NULL == output || NULL == is_leaf || NULL == curr_node_) {
    return false;
  }

  curr_node_ = input ? curr_node_->right : curr_node_->left;
  if (NULL == curr_node_) {
    return false;
  }

  *is_leaf =
      (NULL == curr_node_->left && NULL == curr_node_->right) ? true : false;
  if (*is_leaf) {
    *output = curr_node_->symbol;
    curr_node_ = root_;
  }

  return true;
}

}  // namespace sptk
