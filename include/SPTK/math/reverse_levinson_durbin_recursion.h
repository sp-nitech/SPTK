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
