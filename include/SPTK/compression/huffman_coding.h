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
