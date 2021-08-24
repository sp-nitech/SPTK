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

#include "SPTK/compression/huffman_coding.h"

#include <cstddef>  // std::size_t
#include <queue>    // std::priority_queue

namespace {

class Node {
 public:
  /**
   * Leaf node.
   */
  Node(int symbol, double probability)
      : symbol_(symbol), probability_(probability), left_(NULL), right_(NULL) {
  }

  /**
   * Internal node.
   */
  Node(const Node* left, const Node* right)
      : symbol_(-1),
        probability_((NULL == left || NULL == right)
                         ? 0.0
                         : left->GetProbability() + right->GetProbability()),
        left_(left),
        right_(right) {
  }

  virtual ~Node() {
  }

  int GetSymbol() const {
    return symbol_;
  }

  double GetProbability() const {
    return probability_;
  }

  const Node* GetLeft() const {
    return left_;
  }

  const Node* GetRight() const {
    return right_;
  }

 private:
  const int symbol_;
  const double probability_;
  const Node* left_;
  const Node* right_;

  DISALLOW_COPY_AND_ASSIGN(Node);
};

struct Compare {
  bool operator()(const Node* a, const Node* b) const {
    return b->GetProbability() < a->GetProbability();
  }
};

void Encode(const Node* node, std::string code,
            std::vector<std::string>* codewords) {
  if (NULL == node) return;

  const int symbol(node->GetSymbol());
  if (0 <= symbol) {
    (*codewords)[symbol] = code;
  } else {
    Encode(node->GetLeft(), code + "0", codewords);
    Encode(node->GetRight(), code + "1", codewords);
  }
}

void Free(const Node* node) {
  if (NULL == node) return;
  Free(node->GetLeft());
  Free(node->GetRight());
  delete node;
}

}  // namespace

namespace sptk {

HuffmanCoding::HuffmanCoding(int num_element)
    : num_element_(num_element), is_valid_(true) {
  if (num_element_ <= 0) {
    is_valid_ = false;
    return;
  }
}

bool HuffmanCoding::Run(const std::vector<double>& probabilities,
                        std::vector<std::string>* codewords) const {
  // Check inputs.
  if (!is_valid_ ||
      probabilities.size() != static_cast<std::size_t>(num_element_) ||
      NULL == codewords) {
    return false;
  }

  // Prepare memories.
  if (codewords->size() != static_cast<std::size_t>(num_element_)) {
    codewords->resize(num_element_);
  }

  if (1 == num_element_) {
    (*codewords)[0] = "0";
    return true;
  }

  std::priority_queue<Node*, std::vector<Node*>, Compare> tree;

  try {
    for (int i(0); i < num_element_; ++i) {
      tree.push(new Node(i, probabilities[i]));
    }
    while (1 < tree.size()) {
      const Node* left(tree.top());
      tree.pop();
      const Node* right(tree.top());
      tree.pop();

      try {
        tree.push(new Node(left, right));
      } catch (...) {
        delete left;
        delete right;
        throw;
      }
    }
    Encode(tree.top(), "", codewords);
    Free(tree.top());
  } catch (...) {
    while (!tree.empty()) {
      Free(tree.top());
      tree.pop();
    }
    return false;
  }

  return true;
}

}  // namespace sptk
