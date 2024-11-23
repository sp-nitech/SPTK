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

#ifndef SPTK_MATH_PRODUCT_ACCUMULATION_H_
#define SPTK_MATH_PRODUCT_ACCUMULATION_H_

#include <algorithm>  // std::fill
#include <vector>     // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Accumulate product of input vectors.
 *
 * The input of is an @f$M@f$-th order vector:
 * @f[
 *   \begin{array}{cccc}
 *     x_t(0), & x_t(1), & \ldots, & x_t(M).
 *   \end{array}
 * @f]
 * After running @c Run @f$T@f$ times, the following statistics are obtained:
 * @f{eqnarray}{
 *        P_0 &=& T, \\
 *     P_1(m) &=& \prod_{t=0}^{T-1} x_t(m).
 * @f}
 */
class ProductAccumulation {
 public:
  /**
   * Buffer for ProductAccumulation class.
   */
  class Buffer {
   public:
    Buffer() : num_data_(0) {
    }

    virtual ~Buffer() {
    }

   private:
    void Clear() {
      num_data_ = 0;
      std::fill(interim_.begin(), interim_.end(), 1.0);
    }

    int num_data_;
    std::vector<double> interim_;

    friend class ProductAccumulation;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_order Order of vector, @f$M@f$.
   * @param[in] numerically_stable If true, use a numerically stable algorithm.
   */
  explicit ProductAccumulation(int num_order, bool numerically_stable = false);

  virtual ~ProductAccumulation() {
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
   * @param[in] buffer Buffer.
   * @param[out] num_data Number of accumulated data.
   * @return True on success, false on failure.
   */
  bool GetNumData(const ProductAccumulation::Buffer& buffer,
                  int* num_data) const;

  /**
   * @param[in] buffer Buffer.
   * @param[out] product Product of accumulated data.
   * @return True on success, false on failure.
   */
  bool GetProduct(const ProductAccumulation::Buffer& buffer,
                  std::vector<double>* product) const;

  /**
   * @param[in] buffer Buffer.
   * @param[out] mean Geometric mean of accumulated data.
   * @return True on success, false on failure.
   */
  bool GetGeometricMean(const ProductAccumulation::Buffer& buffer,
                        std::vector<double>* mean) const;

  /**
   * Clear buffer.
   *
   * @param[in,out] buffer Buffer.
   */
  void Clear(ProductAccumulation::Buffer* buffer) const;

  /**
   * Accumulate product.
   *
   * @param[in] data @f$M@f$-th order input vector.
   * @param[in,out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& data,
           ProductAccumulation::Buffer* buffer) const;

 private:
  const int num_order_;
  const bool numerically_stable_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(ProductAccumulation);
};

}  // namespace sptk

#endif  // SPTK_MATH_PRODUCT_ACCUMULATION_H_
