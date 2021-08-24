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

#ifndef SPTK_COMPRESSION_INVERSE_MULTISTAGE_VECTOR_QUANTIZATION_H_
#define SPTK_COMPRESSION_INVERSE_MULTISTAGE_VECTOR_QUANTIZATION_H_

#include <vector>  // std::vector

#include "SPTK/compression/inverse_vector_quantization.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Perform inverse multistage vector quantization.
 *
 * The input is the @f$N@f$ indices of codebook vectors:
 * @f[
 *   \begin{array}{cccc}
 *     i(1), & i(2), & \ldots, & i(N),
 *   \end{array}
 * @f]
 * and the @f$M@f$-th order @f$N \times I@f$ codebook vectors,
 * @f$\left\{ c_i^{(n)}(m) \right\}@f$.
 * The output is the @f$M@f$-th order reconstructed vector:
 * @f[
 *   \begin{array}{cccc}
 *     x^{(N)}(0), & x^{(N)}(1), & \ldots, & x^{(N)}(M).
 *   \end{array}
 * @f]
 * The reconstructed vector is obtained by the recursion:
 * @f[
 *   x^{(n)}(m) = c_{i(n)}^{(n)}(m) + x^{(n-1)}(m),
 * @f]
 * where @f$x^{(0)}(m) = 0@f$ for any @f$m@f$.
 */
class InverseMultistageVectorQuantization {
 public:
  /**
   * Buffer for InverseMultistageVectorQuantization class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    std::vector<double> quantization_error_;

    friend class InverseMultistageVectorQuantization;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_order Order of vector, @f$M@f$.
   * @param[in] num_stage Number of quantization stages, @f$N@f$.
   */
  InverseMultistageVectorQuantization(int num_order, int num_stage);

  virtual ~InverseMultistageVectorQuantization() {
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
   * @param[in] codebook_indices @f$N@f$ codebook indices.
   * @param[in] codebook_vectors @f$M@f$-th order @f$I@f$ codebook vectors.
   *            The shape is @f$[N, I, M+1]@f$.
   * @param[out] reconstructed_vector @f$M@f$-th order output vector.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(
      const std::vector<int>& codebook_indices,
      const std::vector<std::vector<std::vector<double> > >& codebook_vectors,
      std::vector<double>* reconstructed_vector,
      InverseMultistageVectorQuantization::Buffer* buffer) const;

 private:
  const int num_order_;
  const int num_stage_;
  const InverseVectorQuantization inverse_vector_quantization_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(InverseMultistageVectorQuantization);
};

}  // namespace sptk

#endif  // SPTK_COMPRESSION_INVERSE_MULTISTAGE_VECTOR_QUANTIZATION_H_
