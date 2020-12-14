// ----------------------------------------------------------------- //
//             The Speech Signal Processing Toolkit (SPTK)           //
//             developed by SPTK Working Group                       //
//             http://sp-tk.sourceforge.net/                         //
// ----------------------------------------------------------------- //
//                                                                   //
//  Copyright (c) 1984-2007  Tokyo Institute of Technology           //
//                           Interdisciplinary Graduate School of    //
//                           Science and Engineering                 //
//                                                                   //
//                1996-2020  Nagoya Institute of Technology          //
//                           Department of Computer Science          //
//                                                                   //
// All rights reserved.                                              //
//                                                                   //
// Redistribution and use in source and binary forms, with or        //
// without modification, are permitted provided that the following   //
// conditions are met:                                               //
//                                                                   //
// - Redistributions of source code must retain the above copyright  //
//   notice, this list of conditions and the following disclaimer.   //
// - Redistributions in binary form must reproduce the above         //
//   copyright notice, this list of conditions and the following     //
//   disclaimer in the documentation and/or other materials provided //
//   with the distribution.                                          //
// - Neither the name of the SPTK working group nor the names of its //
//   contributors may be used to endorse or promote products derived //
//   from this software without specific prior written permission.   //
//                                                                   //
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            //
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       //
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          //
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          //
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS //
// BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          //
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   //
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     //
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON //
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   //
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    //
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           //
// POSSIBILITY OF SUCH DAMAGE.                                       //
// ----------------------------------------------------------------- //

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
