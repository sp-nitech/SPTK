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
