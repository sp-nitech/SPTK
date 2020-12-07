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

#ifndef SPTK_COMPRESSION_INVERSE_VECTOR_QUANTIZATION_H_
#define SPTK_COMPRESSION_INVERSE_VECTOR_QUANTIZATION_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Perform inverse vector quantization.
 *
 * The input is the index of a codebook vector @f$i@f$ and the @f$M@f$-th order
 * codebook vectors:
 * @f[
 *   \begin{array}{cccc}
 *     \boldsymbol{c}_0, & \boldsymbol{c}_1, & \ldots, & \boldsymbol{c}_{I-1}.
 *   \end{array}
 * @f]
 * The output is simply the @f$i@f$-th codebook vector @f$\boldsymbol{c}_i@f$.
 */
class InverseVectorQuantization {
 public:
  /**
   * @param[in] num_order Order of vector, @f$M@f$.
   */
  explicit InverseVectorQuantization(int num_order);

  virtual ~InverseVectorQuantization() {
  }

  /**
   * @return Order of vector.
   */
  int GetNumOrder() const {
    return num_order_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] codebook_index Codebook index.
   * @param[in] codebook_vectors @f$M@f$-th order @f$I@f$ codebook vectors.
   *            The shape is @f$[I, M]@f$.
   * @param[out] reconstructed_vector @f$M@f$-th order codebook vector.
   * @return True on success, false on failure.
   */
  bool Run(int codebook_index,
           const std::vector<std::vector<double> >& codebook_vectors,
           std::vector<double>* reconstructed_vector) const;

 private:
  const int num_order_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(InverseVectorQuantization);
};

}  // namespace sptk

#endif  // SPTK_COMPRESSION_INVERSE_VECTOR_QUANTIZATION_H_
