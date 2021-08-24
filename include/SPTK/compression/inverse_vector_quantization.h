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
   *            The shape is @f$[I, M+1]@f$.
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
