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

#ifndef SPTK_MATH_REVERSE_LEVINSON_DURBIN_RECURSION_H_
#define SPTK_MATH_REVERSE_LEVINSON_DURBIN_RECURSION_H_

#include <vector>  // std::vector

#include "SPTK/math/symmetric_matrix.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Calculate linear predictive coefficients from autocorrelation.
 *
 * The input is the @f$M@f$-th order LPC coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     K, & a(1), & \ldots, & a(M),
 *   \end{array}
 * @f]
 * where @f$K@f$ is the gain, and the output is the @f$M@f$-th order
 * autocorrelation:
 * @f[
 *   \begin{array}{cccc}
 *     r(0), & r(1), & \ldots, & r(M).
 *   \end{array}
 * @f]
 * The autocorrelation matrix can be represented as
 * @f[
 *   \boldsymbol{R} = \left[ \begin{array}{cccc}
 *     r(0)   & r(1)   & \cdots & r(M) \\
 *     r(1)   & r(0)   & \cdots & r(M-1) \\
 *     \vdots & \vdots & \ddots & \vdots \\
 *     r(M)   & r(M-1) & \cdots & r(0)
 *   \end{array} \right].
 * @f]
 * The autocorrelation is derived by using the matrix decomposition
 * @f[
 *   \boldsymbol{R}^{-1} = \boldsymbol{U} \boldsymbol{E}^{-1}
 *                         \boldsymbol{U}^{\mathsf{T}}.
 * @f]
 * The @f$\boldsymbol{U}@f$ is the following upper triangular matrix:
 * @f[
 *   \boldsymbol{U} = \left[ \begin{array}{cccc}
 *     a^{(0)}(0) & a^{(1)}(1) & \cdots & a^{(M)}(M) \\
 *     0          & a^{(1)}(0) & \cdots & a^{(M)}(M-1) \\
 *     \vdots     & \vdots     & \ddots & \vdots \\
 *     0          & 0          & \cdots & a^{(M)}(0)
 *   \end{array} \right],
 * @f]
 * where @f$a^{(i)}(j)@f$ is the @f$j@f$-th coefficient of the @f$i@f$-th order
 * prediction filter polynomial.
 * The @f$\boldsymbol{E}@f$ is the following diagonal matrix:
 * @f[
 *   \boldsymbol{E} = \left[ \begin{array}{cccc}
 *     e^{(0)}(0) & 0          & \cdots & 0 \\
 *     0          & e^{(1)}(1) & \cdots & 0 \\
 *     \vdots     & \vdots     & \ddots & \vdots \\
 *     0          & 0          & \cdots & e^{(M)}(M)
 *   \end{array} \right],
 * @f]
 * where @f$e^{(i)}(i)@f$ is the prediction error from @f$i@f$-th order filter.
 * This decomposition allows us the efficient evaluation of the inverse of the
 * autocorrelation matrix.
 */
class ReverseLevinsonDurbinRecursion {
 public:
  /**
   * Buffer for ReverseLevinsonDurbinRecursion class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    SymmetricMatrix u_;
    std::vector<double> e_;

    friend class ReverseLevinsonDurbinRecursion;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_order Order of coefficients, @f$M@f$.
   */
  explicit ReverseLevinsonDurbinRecursion(int num_order);

  virtual ~ReverseLevinsonDurbinRecursion() {
  }

  /**
   * @return Order of coefficients.
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
   * @param[in] linear_predictive_coefficients @f$M@f$-th order LPC
   *            coefficients.
   * @param[out] autocorrelation @f$M@f$-th order autocorrelation.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& linear_predictive_coefficients,
           std::vector<double>* autocorrelation,
           ReverseLevinsonDurbinRecursion::Buffer* buffer) const;

  /**
   * @param[in,out] input_and_output @f$M@f$-th order coefficients.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(std::vector<double>* input_and_output,
           ReverseLevinsonDurbinRecursion::Buffer* buffer) const;

 private:
  const int num_order_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(ReverseLevinsonDurbinRecursion);
};

}  // namespace sptk

#endif  // SPTK_MATH_REVERSE_LEVINSON_DURBIN_RECURSION_H_
