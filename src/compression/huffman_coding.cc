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
//                1996-2021  Nagoya Institute of Technology          //
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
