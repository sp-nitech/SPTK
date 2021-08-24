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
