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

#ifndef SPTK_CONVERSION_LINEAR_PREDICTIVE_COEFFICIENTS_TO_PARCOR_COEFFICIENTS_H_
#define SPTK_CONVERSION_LINEAR_PREDICTIVE_COEFFICIENTS_TO_PARCOR_COEFFICIENTS_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Convert LPC coefficients to PARCOR coefficients.
 *
 * The input is the @f$M@f$-th order LPC coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     K, & a(1), & \ldots, & a(M),
 *   \end{array}
 * @f]
 * and the output is the @f$M@f$-th order PARCOR coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     K, & k(1), & \ldots, & k(M),
 *   \end{array}
 * @f]
 * where @f$K@f$ is the gain. The conversion is given by the following
 * recursion formula:
 * @f{eqnarray}{
 *   k(i) &=& a^{(i)}(i), \\
 *   a^{(i-1)}(m) &=& \frac{a^{(i)}(m) + a^{(i)}(i) a^{(i)}(i-m)}{1-k^2(i)}, \\
 *   && i = M,\ldots,1
 * @f}
 * with the initial condition @f$a^{(M)}(i)=a(i)@f$ for @f$i = 1,\ldots,M@f$.
 *
 * The input can be the @f$M@f$-th order normalized generalized cepstral
 * coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     K, & c'_\gamma(1), & \ldots, & c'_\gamma(M).
 *   \end{array}
 * @f]
 * In the case, the initial condition is @f$a^{(M)}(i)=\gamma \, c'_\gamma(i)@f$
 * for @f$i = 1,\ldots,M@f$.
 */
class LinearPredictiveCoefficientsToParcorCoefficients {
 public:
  /**
   * Buffer for LinearPredictiveCoefficientsToParcorCoefficients class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    std::vector<double> a_;

    friend class LinearPredictiveCoefficientsToParcorCoefficients;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_order Order of coefficients.
   * @param[in] gamma Gamma.
   */
  LinearPredictiveCoefficientsToParcorCoefficients(int num_order, double gamma);

  virtual ~LinearPredictiveCoefficientsToParcorCoefficients() {
  }

  /**
   * @return Order of coefficients.
   */
  int GetNumOrder() const {
    return num_order_;
  }

  /**
   * @return Gamma.
   */
  double GetGamma() const {
    return gamma_;
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
   * @param[out] parcor_coefficients @f$M@f$-th order PARCOR coefficients.
   * @param[out] is_stable True if given coefficients are stable.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(
      const std::vector<double>& linear_predictive_coefficients,
      std::vector<double>* parcor_coefficients, bool* is_stable,
      LinearPredictiveCoefficientsToParcorCoefficients::Buffer* buffer) const;

  /**
   * @param[in,out] input_and_output @f$M@f$-th order coefficients.
   * @param[out] is_stable True if given coefficients are stable.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(
      std::vector<double>* input_and_output, bool* is_stable,
      LinearPredictiveCoefficientsToParcorCoefficients::Buffer* buffer) const;

 private:
  const int num_order_;
  const double gamma_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(LinearPredictiveCoefficientsToParcorCoefficients);
};

}  // namespace sptk

#endif  // SPTK_CONVERSION_LINEAR_PREDICTIVE_COEFFICIENTS_TO_PARCOR_COEFFICIENTS_H_
