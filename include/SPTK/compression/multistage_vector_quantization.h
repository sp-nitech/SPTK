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

#ifndef SPTK_COMPRESSION_MULTISTAGE_VECTOR_QUANTIZATION_H_
#define SPTK_COMPRESSION_MULTISTAGE_VECTOR_QUANTIZATION_H_

#include <vector>  // std::vector

#include "SPTK/compression/vector_quantization.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Perform multistage vector quantization.
 *
 * The input is the @f$M@f$-th order vector:
 * @f[
 *   \begin{array}{cccc}
 *     x(0), & x(1), & \ldots, & x(M),
 *   \end{array}
 * @f]
 * and the @f$M@f$-th order @f$N \times I@f$ codebook vectors,
 * @f$\left\{ c_i^{(n)}(m) \right\}@f$.
 * The output is the @f$N@f$ codebook indices:
 * @f[
 *   \begin{array}{cccc}
 *     i(1), & i(2), & \ldots, & i(N),
 *   \end{array}
 * @f]
 * where
 * @f[
 *   i(n) = \mathop{\mathrm{argmin}}_j \sum_{j=0}^{I-1} \sum_{m=0}^M
 *       (e^{(n)}(m) - c_j^{(n)}(m))^2,
 * @f]
 * and the quantization error is
 * @f[
 *   e^{(n)}(m) = \left\{ \begin{array}{ll}
 *     x(m), & n = 1 \\
 *     e^{(n-1)}(m) - c_j^{(n-1)}(m). & n > 1 \\
 *   \end{array} \right.
 * @f]
 */
class MultistageVectorQuantization {
 public:
  /**
   * Buffer for MultistageVectorQuantization class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    std::vector<double> quantization_error_;

    friend class MultistageVectorQuantization;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_order Order of vector, @f$M@f$.
   * @param[in] num_stage Number of quantization stages, @f$N@f$.
   */
  MultistageVectorQuantization(int num_order, int num_stage);

  virtual ~MultistageVectorQuantization() {
  }

  /**
   * @return Order of vector.
   */
  int GetNumOrder() const {
    return num_order_;
  }

  /**
   * @return Number of stages.
   */
  int GetNumStage() const {
    return num_stage_;
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
   *             The shape is @f$[N, I, M+1]@f$.
   * @param[out] codebook_indices @f$N@f$ codebook indices.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(
      const std::vector<double>& input_vector,
      const std::vector<std::vector<std::vector<double> > >& codebook_vectors,
      std::vector<int>* codebook_indices,
      MultistageVectorQuantization::Buffer* buffer) const;

 private:
  const int num_order_;
  const int num_stage_;
  const VectorQuantization vector_quantization_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(MultistageVectorQuantization);
};

}  // namespace sptk

#endif  // SPTK_COMPRESSION_MULTISTAGE_VECTOR_QUANTIZATION_H_
