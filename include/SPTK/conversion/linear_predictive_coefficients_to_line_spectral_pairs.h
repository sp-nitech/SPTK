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

#ifndef SPTK_CONVERSION_LINEAR_PREDICTIVE_COEFFICIENTS_TO_LINE_SPECTRAL_PAIRS_H_
#define SPTK_CONVERSION_LINEAR_PREDICTIVE_COEFFICIENTS_TO_LINE_SPECTRAL_PAIRS_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Convert linear predictive coefficients to line spectral pairs.
 *
 * The input is the @f$M@f$-th order LPC:
 * @f[
 *   \begin{array}{cccc}
 *     K, & a(1), & \ldots, & a(M),
 *   \end{array}
 * @f]
 * and the output is the @f$M@f$-th order LSP:
 * @f[
 *   \begin{array}{cccc}
 *     K, & \omega(1), & \ldots, & \omega(M).
 *   \end{array}
 * @f]
 * The LSP coefficients are obtained by an iterative root finding algorithm.
 *
 * [1] P. Kabal and R. P. Ramachandran, &quot;The computation of line spectral
 *     frequencies using Chebyshev polynomials,&quot; IEEE Transactions on
 *     Acoustics, Speech, and Signal Processing, vol. 34, no. 6, pp. 1419-1426,
 *     1986.
 */
class LinearPredictiveCoefficientsToLineSpectralPairs {
 public:
  /**
   * Buffer for LinearPredictiveCoefficientsToLineSpectralPairs class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    std::vector<double> c1_;
    std::vector<double> c2_;

    friend class LinearPredictiveCoefficientsToLineSpectralPairs;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_order Order of coefficients, @f$M@f$.
   * @param[in] num_split Number of splits of unit circle.
   * @param[in] num_iteration Number of iterations.
   * @param[in] convergence_threshold Convergence threshold.
   */
  LinearPredictiveCoefficientsToLineSpectralPairs(int num_order, int num_split,
                                                  int num_iteration,
                                                  double convergence_threshold);

  virtual ~LinearPredictiveCoefficientsToLineSpectralPairs() {
  }

  /**
   * @return Order of coefficients.
   */
  int GetNumOrder() const {
    return num_order_;
  }

  /**
   * @return Number of splits.
   */
  int GetNumSplit() const {
    return num_split_;
  }

  /**
   * @return Number of iterations.
   */
  int GetNumIteration() const {
    return num_iteration_;
  }

  /**
   * @return Convergence threshold.
   */
  double GetConvergenceThreshold() const {
    return convergence_threshold_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] linear_predictive_coefficients @f$M@f$-th order LPC.
   * @param[out] line_spectral_pairs @f$M@f$-th order LSP.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(
      const std::vector<double>& linear_predictive_coefficients,
      std::vector<double>* line_spectral_pairs,
      LinearPredictiveCoefficientsToLineSpectralPairs::Buffer* buffer) const;

  /**
   * @param[in,out] input_and_output @f$M@f$-th order coefficients.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(
      std::vector<double>* input_and_output,
      LinearPredictiveCoefficientsToLineSpectralPairs::Buffer* buffer) const;

 private:
  const int num_order_;
  const int num_symmetric_polynomial_order_;
  const int num_asymmetric_polynomial_order_;
  const int num_split_;
  const int num_iteration_;
  const double convergence_threshold_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(LinearPredictiveCoefficientsToLineSpectralPairs);
};

}  // namespace sptk

#endif  // SPTK_CONVERSION_LINEAR_PREDICTIVE_COEFFICIENTS_TO_LINE_SPECTRAL_PAIRS_H_
