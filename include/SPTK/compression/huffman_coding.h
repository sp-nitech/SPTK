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

#ifndef SPTK_COMPRESSION_HUFFMAN_CODING_H_
#define SPTK_COMPRESSION_HUFFMAN_CODING_H_

#include <string>  // std::string
#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Generate variable-length code codewords based on Huffman coding.
 *
 * The input is the probabilites or frequencies of @f$N@f$ events:
 * @f[
 *   \begin{array}{cccc}
 *     p(0), & p(1), & \ldots, & p(N-1),
 *   \end{array}
 * @f]
 * and the output is the corresponding codewords:
 * @f[
 *   \begin{array}{cccc}
 *     c(0), & c(1), & \ldots, & c(N-1),
 *   \end{array}
 * @f]
 * where @f$c(n) \in \{0,1\}^{\ast}@f$.
 *
 * The implementation is based on priority queue.
 */
class HuffmanCoding {
 public:
  /**
   * @param[in] num_element Number of elements.
   */
  explicit HuffmanCoding(int num_element);

  virtual ~HuffmanCoding() {
  }

  /**
   * @return Number of elements.
   */
  int GetNumElement() const {
    return num_element_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] probabilities @f$N@f$ probabilities or frequencies.
   * @param[out] codewords @f$N@f$ codewords.
   */
  bool Run(const std::vector<double>& probabilities,
           std::vector<std::string>* codewords) const;

 private:
  const int num_element_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(HuffmanCoding);
};

}  // namespace sptk

#endif  // SPTK_COMPRESSION_HUFFMAN_CODING_H_
