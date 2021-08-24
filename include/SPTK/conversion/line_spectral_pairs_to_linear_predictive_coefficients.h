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

#ifndef SPTK_CONVERSION_LINE_SPECTRAL_PAIRS_TO_LINEAR_PREDICTIVE_COEFFICIENTS_H_
#define SPTK_CONVERSION_LINE_SPECTRAL_PAIRS_TO_LINEAR_PREDICTIVE_COEFFICIENTS_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Convert line spectral pairs to linear predictive coefficients.
 *
 * The input is the @f$M@f$-th order LSP:
 * @f[
 *   \begin{array}{cccc}
 *     K, & \omega(1), & \ldots, & \omega(M),
 *   \end{array}
 * @f]
 * and the output is the @f$M@f$-th order LPC:
 * @f[
 *   \begin{array}{cccc}
 *     K, & a(1), & \ldots, & a(M).
 *   \end{array}
 * @f]
 * The LSP coefficients are the roots of the following polynomials:
 * @f{eqnarray}{
 *   P(z)&=&(1-z^{-1})\prod_{m=2,4,\ldots}^M (1-2\cos\omega(m)z^{-1}+z^{-2}), \\
 *   Q(z)&=&(1+z^{-1})\prod_{m=1,3,\ldots}^M (1-2\cos\omega(m)z^{-1}+z^{-2}).
 * @f}
 * The relation between the polynomials and the all-pole filter is
 * @f{eqnarray}{
 *   A(z) &=& 1 + \sum_{m=1}^M a(m) z^{-m} \\
 *        &=& \displaystyle\frac{P(z) + Q(z)}{2}.
 * @f}
 * The LPC coefficients are derived from the LSP coefficients using the above
 * relation.
 */
class LineSpectralPairsToLinearPredictiveCoefficients {
 public:
  /**
   * Buffer for LineSpectralPairsToLinearPredictiveCoefficients class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    std::vector<double> p_;
    std::vector<double> q_;
    std::vector<double> a0_;
    std::vector<double> a1_;
    std::vector<double> a2_;
    std::vector<double> b0_;
    std::vector<double> b1_;
    std::vector<double> b2_;

    friend class LineSpectralPairsToLinearPredictiveCoefficients;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_order Order of coefficients, @f$M@f$.
   */
  explicit LineSpectralPairsToLinearPredictiveCoefficients(int num_order);

  virtual ~LineSpectralPairsToLinearPredictiveCoefficients() {
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
   * @param[in] line_spectral_pairs @f$M@f$-th order LSP.
   * @param[out] linear_predictive_coefficients @f$M@f$-th order LPC.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(
      const std::vector<double>& line_spectral_pairs,
      std::vector<double>* linear_predictive_coefficients,
      LineSpectralPairsToLinearPredictiveCoefficients::Buffer* buffer) const;

  /**
   * @param[in,out] input_and_output @f$M@f$-th order coefficients.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(
      std::vector<double>* input_and_output,
      LineSpectralPairsToLinearPredictiveCoefficients::Buffer* buffer) const;

 private:
  const int num_order_;
  const int num_symmetric_polynomial_order_;
  const int num_asymmetric_polynomial_order_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(LineSpectralPairsToLinearPredictiveCoefficients);
};

}  // namespace sptk

#endif  // SPTK_CONVERSION_LINE_SPECTRAL_PAIRS_TO_LINEAR_PREDICTIVE_COEFFICIENTS_H_
