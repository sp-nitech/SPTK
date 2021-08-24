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

#ifndef SPTK_MATH_LEVINSON_DURBIN_RECURSION_H_
#define SPTK_MATH_LEVINSON_DURBIN_RECURSION_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Calculate linear predictive coefficients from autocorrelation.
 *
 * The input is the @f$M@f$-th order autocorrelation:
 * @f[
 *   \begin{array}{cccc}
 *     r(0), & r(1), & \ldots, & r(M),
 *   \end{array}
 * @f]
 * and the output is the @f$M@f$-th order LPC coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     K, & a(1), & \ldots, & a(M),
 *   \end{array}
 * @f]
 * where @f$K@f$ is the gain.
 * The LPC coefficients are obtained by solving the following set of linear
 * equations:
 * @f[
 *   \left[ \begin{array}{cccc}
 *     r(0)   & r(1)   & \cdots & r(M-1) \\
 *     r(1)   & r(0)   & \cdots & r(M-2) \\
 *     \vdots & \vdots & \ddots & \vdots \\
 *     r(M-1) & r(M-2) & \cdots & r(0)
 *   \end{array} \right]
 *   \left[ \begin{array}{c}
 *     a(1) \\
 *     a(2) \\
 *     \vdots \\
 *     a(M)
 *   \end{array} \right]
 *   = - \left[ \begin{array}{c}
 *     r(1) \\
 *     r(2) \\
 *     \vdots \\
 *     r(M)
 *   \end{array} \right].
 * @f]
 * The Durbin iterative and efficient algorithm is used to solve the above
 * system by taking the addvantage of the Toeplitz characteristic of the
 * autocorrelation matrix:
 * @f{eqnarray}{
 *   k(i) &=& \frac{-r(i)-\displaystyle\sum_{j=1}^i a^{(i-1)}(j)r(i-j)}
 *                 {E^{(i-1)}}, \\
 *   a^{(i)}(j) &=& a^{(i-1)}(j) + k(i) a^{(i-1)}(i-j), \quad (1 \le j < i) \\
 *   a^{(i)}(i) &=& k(i), \\
 *   E^{(i)} &=& (1-k^2(i)) E^{(i-1)}, \\
 *   && \qquad \qquad \qquad \qquad i = 1,2,\ldots,M
 * @f}
 * where the initial condition is @f$E^{(0)} = r(0)@f$ and @f$a^{(0)}(1) = 0@f$.
 * The gain @f$K@f$ is calculated as
 * @f[
 *   K = \sqrt{E^{(M)}}.
 * @f]
 */
class LevinsonDurbinRecursion {
 public:
  /**
   * Buffer for LevinsonDurbinRecursion class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    std::vector<double> c_;

    friend class LevinsonDurbinRecursion;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_order Order of coefficients, @f$M@f$.
   */
  explicit LevinsonDurbinRecursion(int num_order);

  virtual ~LevinsonDurbinRecursion() {
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
   * @param[in] autocorrelation @f$M@f$-th order autocorrelation.
   * @param[out] linear_predictive_coefficients @f$M@f$-th order LPC
   *             coefficients.
   * @param[out] is_stable True if the obtained coefficients are stable.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& autocorrelation,
           std::vector<double>* linear_predictive_coefficients, bool* is_stable,
           LevinsonDurbinRecursion::Buffer* buffer) const;

  /**
   * @param[in,out] input_and_output @f$M@f$-th order coefficients.
   * @param[out] is_stable True if the obtained coefficients are stable.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(std::vector<double>* input_and_output, bool* is_stable,
           LevinsonDurbinRecursion::Buffer* buffer) const;

 private:
  const int num_order_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(LevinsonDurbinRecursion);
};

}  // namespace sptk

#endif  // SPTK_MATH_LEVINSON_DURBIN_RECURSION_H_
