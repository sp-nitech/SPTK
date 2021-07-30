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
//                1996-2021  Nagoya Institute of Technology          //
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
