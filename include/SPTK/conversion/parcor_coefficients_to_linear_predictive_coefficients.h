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

#ifndef SPTK_CONVERSION_PARCOR_COEFFICIENTS_TO_LINEAR_PREDICTIVE_COEFFICIENTS_H_
#define SPTK_CONVERSION_PARCOR_COEFFICIENTS_TO_LINEAR_PREDICTIVE_COEFFICIENTS_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Transform PARCOR coefficients to LPC coefficients.
 *
 * The input is the \f$M\f$-th order PARCOR coefficients:
 * \f[
 *   \begin{array}{cccc}
 *     K, & k(1), & \ldots, & k(M),
 *   \end{array}
 * \f]
 * and the output is the \f$M\f$-th order LPC coefficients:
 * \f[
 *   \begin{array}{cccc}
 *     K, & a(1), & \ldots, & a(M),
 *   \end{array}
 * \f]
 * where \f$K\f$ is the gain. The transformation is given by the following
 * recursion formula:
 * \f[
 *   a^{(i)}(m) = a^{(i-1)}(m) + k(i) a^{(i-1)}(i-m) \\
 *   i = 2,\ldots,M
 * \f]
 * with the initial condition \f$a^{(i)}(i)=k(i)\f$ for \f$i = 1,\ldots,M-1\f$.
 * The outputs can then be written as
 * \f[
 *   a(m) = \left\{ \begin{array}{ll}
 *     a^{(M)}(m), & 1 \le m < M \\
 *     k(m). & m = M
 *   \end{array} \right.
 * \f]
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
   * @return True if this obejct is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] parcor_coefficients \f$M\f$-th order PARCOR coefficients.
   * @param[out] linear_predictive_coefficients \f$M\f$-th order LPC
   *             coefficients.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(
      const std::vector<double>& parcor_coefficients,
      std::vector<double>* linear_predictive_coefficients,
      ParcorCoefficientsToLinearPredictiveCoefficients::Buffer* buffer) const;

  /**
   * @param[in,out] input_and_output \f$M\f$-th order coefficients.
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
