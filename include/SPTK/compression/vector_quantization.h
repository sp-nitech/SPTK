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

#ifndef SPTK_COMPRESSION_VECTOR_QUANTIZATION_H_
#define SPTK_COMPRESSION_VECTOR_QUANTIZATION_H_

#include <vector>  // std::vector

#include "SPTK/math/distance_calculation.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Perform vector quantization.
 *
 * The input is the @f$M@f$-th order vector:
 * @f[
 *   \begin{array}{cccc}
 *     x(0), & x(1), & \ldots, & x(M),
 *   \end{array}
 * @f]
 * and the @f$M@f$-th order codebook vectors:
 * @f[
 *   \begin{array}{cccc}
 *     \boldsymbol{c}_0, & \boldsymbol{c}_1, & \ldots, & \boldsymbol{c}_{I-1}.
 *   \end{array}
 * @f]
 * The output is the index of the codebook vector that minimizes the distance
 * between the input vector and the codebook vector in an Euclidean sense:
 * @f[
 *   \mathop{\mathrm{argmin}}_i \sum_{i=0}^{I-1} \sum_{m=0}^M (x(m) - c_i(m))^2.
 * @f]
 */
class VectorQuantization {
 public:
  /**
   * @param[in] num_order Order of vector, @f$M@f$.
   */
  explicit VectorQuantization(int num_order);

  virtual ~VectorQuantization() {
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
   * @param[in] input_vector @f$M@f$-th order input vector.
   * @param[in] codebook_vectors @f$M@f$-th order @f$I@f$ codebook vectors.
   *            The shape is @f$[I, M+1]@f$.
   * @param[out] codebook_index Codebook index.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& input_vector,
           const std::vector<std::vector<double> >& codebook_vectors,
           int* codebook_index) const;

 private:
  const int num_order_;
  const DistanceCalculation distance_calculation_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(VectorQuantization);
};

}  // namespace sptk

#endif  // SPTK_COMPRESSION_VECTOR_QUANTIZATION_H_
