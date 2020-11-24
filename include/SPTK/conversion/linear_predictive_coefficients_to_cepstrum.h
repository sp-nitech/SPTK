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

#ifndef SPTK_CONVERSION_LINEAR_PREDICTIVE_COEFFICIENTS_TO_CEPSTRUM_H_
#define SPTK_CONVERSION_LINEAR_PREDICTIVE_COEFFICIENTS_TO_CEPSTRUM_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Convert LPC coefficients to LPC cepstral coefficients.
 *
 * The input is the @f$M_1@f$-th order LPC coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     K, & a(1), & \ldots, & a(M_1),
 *   \end{array}
 * @f]
 * and the output is the @f$M_2@f$-th order cepstral coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     c(0), & c(1), & \ldots, & c(M_2).
 *   \end{array}
 * @f]
 * The cesptral coefficients is obtained by the following recursion:
 * @f[
 *   c(m) = \left\{ \begin{array}{ll}
 *      \log K, & m = 0 \\
 *      -a(m) - \displaystyle\sum_{k=1}^{m-1} \frac{k}{m} c(k) a(m-k),
 *         & (0 < m \le M_1) \\
 *      -\displaystyle\sum_{k=m-M_1}^{m-1} \frac{k}{m} c(k) a(m-k).
 *         & (M_1 < m \le M_2)
 *   \end{array} \right.
 * @f]
 * This simple recursion does not require any DFTs.
 */
class LinearPredictiveCoefficientsToCepstrum {
 public:
  /**
   * @param[in] num_input_order Order of LPC coefficients, @f$M_1@f$.
   * @param[in] num_output_order Order of cepstral coefficients, @f$M_2@f$.
   */
  LinearPredictiveCoefficientsToCepstrum(int num_input_order,
                                         int num_output_order);

  virtual ~LinearPredictiveCoefficientsToCepstrum() {
  }

  /**
   * @return Order of LPC coefficients.
   */
  int GetNumInputOrder() const {
    return num_input_order_;
  }

  /**
   * @return Order of cepstral coefficients.
   */
  int GetNumOutputOrder() const {
    return num_output_order_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] linear_predictive_coefficients @f$M_1@f$-th order LPC
   *            coefficients.
   * @param[out] cepstrum @f$M_2@f$-th order cepstral coefficients.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& linear_predictive_coefficients,
           std::vector<double>* cepstrum) const;

 private:
  const int num_input_order_;
  const int num_output_order_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(LinearPredictiveCoefficientsToCepstrum);
};

}  // namespace sptk

#endif  // SPTK_CONVERSION_LINEAR_PREDICTIVE_COEFFICIENTS_TO_CEPSTRUM_H_
