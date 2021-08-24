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

#ifndef SPTK_CONVERSION_PARCOR_COEFFICIENTS_TO_LINEAR_PREDICTIVE_COEFFICIENTS_H_
#define SPTK_CONVERSION_PARCOR_COEFFICIENTS_TO_LINEAR_PREDICTIVE_COEFFICIENTS_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Convert PARCOR coefficients to LPC coefficients.
 *
 * The input is the @f$M@f$-th order PARCOR coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     K, & k(1), & \ldots, & k(M),
 *   \end{array}
 * @f]
 * and the output is the @f$M@f$-th order LPC coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     K, & a(1), & \ldots, & a(M),
 *   \end{array}
 * @f]
 * where @f$K@f$ is the gain. The conversion is given by the following
 * recursion formula:
 * @f[
 *   a^{(i)}(m) = a^{(i-1)}(m) + k(i) a^{(i-1)}(i-m) \\
 *   i = 2,\ldots,M
 * @f]
 * with the initial condition @f$a^{(i)}(i)=k(i)@f$ for @f$i = 1,\ldots,M-1@f$.
 * The outputs can then be written as
 * @f[
 *   a(m) = \left\{ \begin{array}{ll}
 *     a^{(M)}(m), & 1 \le m < M \\
 *     k(m). & m = M
 *   \end{array} \right.
 * @f]
 */
class ParcorCoefficientsToLinearPredictiveCoefficients {
 public:
  /**
   * Buffer for ParcorCoefficientsToLinearPredictiveCoefficients class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    std::vector<double> a_;

    friend class ParcorCoefficientsToLinearPredictiveCoefficients;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_order Order of coefficients.
   */
  explicit ParcorCoefficientsToLinearPredictiveCoefficients(int num_order);

  virtual ~ParcorCoefficientsToLinearPredictiveCoefficients() {
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
   * @param[in] parcor_coefficients @f$M@f$-th order PARCOR coefficients.
   * @param[out] linear_predictive_coefficients @f$M@f$-th order LPC
   *             coefficients.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(
      const std::vector<double>& parcor_coefficients,
      std::vector<double>* linear_predictive_coefficients,
      ParcorCoefficientsToLinearPredictiveCoefficients::Buffer* buffer) const;

  /**
   * @param[in,out] input_and_output @f$M@f$-th order coefficients.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(
      std::vector<double>* input_and_output,
      ParcorCoefficientsToLinearPredictiveCoefficients::Buffer* buffer) const;

 private:
  const int num_order_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(ParcorCoefficientsToLinearPredictiveCoefficients);
};

}  // namespace sptk

#endif  // SPTK_CONVERSION_PARCOR_COEFFICIENTS_TO_LINEAR_PREDICTIVE_COEFFICIENTS_H_
