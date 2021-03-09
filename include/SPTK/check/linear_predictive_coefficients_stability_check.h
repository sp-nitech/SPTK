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

#ifndef SPTK_CHECK_LINEAR_PREDICTIVE_COEFFICIENTS_STABILITY_CHECK_H_
#define SPTK_CHECK_LINEAR_PREDICTIVE_COEFFICIENTS_STABILITY_CHECK_H_

#include <vector>  // std::vector

#include "SPTK/conversion/linear_predictive_coefficients_to_parcor_coefficients.h"
#include "SPTK/conversion/parcor_coefficients_to_linear_predictive_coefficients.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Check stability of linear predictive coefficients and modify them.
 *
 * The input is the @f$M@f$-th order LPC:
 * @f[
 *   \begin{array}{cccc}
 *     K, & a(1), & \ldots, & a(M),
 *   \end{array}
 * @f]
 * and the output is the modified @f$M@f$-th order LPC:
 * @f[
 *   \begin{array}{cccc}
 *     K, & a'(1), & \ldots, & a'(M).
 *   \end{array}
 * @f]
 * The LPC @f$a(\cdot)@f$ are stable if the PARCOR coefficients @f$k(\cdot)@f$
 * satisfy the following condition:
 * @f[
 *   k(m) \le 1.0 - \delta
 * @f]
 * where @f$\delta@f$ is a small positive value.
 */
class LinearPredictiveCoefficientsStabilityCheck {
 public:
  /**
   * Buffer for LinearPredictiveCoefficientsStabilityCheck.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    LinearPredictiveCoefficientsToParcorCoefficients::Buffer conversion_buffer_;
    ParcorCoefficientsToLinearPredictiveCoefficients::Buffer
        reconversion_buffer_;
    std::vector<double> parcor_coefficients_;

    friend class LinearPredictiveCoefficientsStabilityCheck;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_order Order of LPC, @f$M@f$.
   * @param[in] margin Margin in PARCOR domain, @f$\delta@f$.
   */
  LinearPredictiveCoefficientsStabilityCheck(int num_order, double margin);

  virtual ~LinearPredictiveCoefficientsStabilityCheck() {
  }

  /**
   * @return Order of coefficients.
   */
  int GetNumOrder() const {
    return num_order_;
  }

  /**
   * @return Margin.
   */
  double GetMargin() const {
    return margin_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] linear_predictive_coefficients @f$M@f$-th order LPC.
   * @param[out] modified_linear_predictive_coefficients Modified @f$M@f$-th
   *             order LPC (optional).
   * @param[out] is_stable True if the given coefficients are stable.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& linear_predictive_coefficients,
           std::vector<double>* modified_linear_predictive_coefficients,
           bool* is_stable,
           LinearPredictiveCoefficientsStabilityCheck::Buffer* buffer) const;

  /**
   * @param[in,out] input_and_output @f$M@f$-th order coefficients.
   * @param[out] is_stable True if the given coefficients are stable.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(std::vector<double>* input_and_output, bool* is_stable,
           LinearPredictiveCoefficientsStabilityCheck::Buffer* buffer) const;

 private:
  const int num_order_;
  const double margin_;

  const LinearPredictiveCoefficientsToParcorCoefficients
      linear_predictive_coefficients_to_parcor_coefficients_;
  const ParcorCoefficientsToLinearPredictiveCoefficients
      parcor_coefficients_to_linear_predictive_coefficients_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(LinearPredictiveCoefficientsStabilityCheck);
};

}  // namespace sptk

#endif  // SPTK_CHECK_LINEAR_PREDICTIVE_COEFFICIENTS_STABILITY_CHECK_H_
