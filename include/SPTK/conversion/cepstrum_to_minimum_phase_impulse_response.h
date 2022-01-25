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

#ifndef SPTK_CONVERSION_CEPSTRUM_TO_MINIMUM_PHASE_IMPULSE_RESPONSE_H_
#define SPTK_CONVERSION_CEPSTRUM_TO_MINIMUM_PHASE_IMPULSE_RESPONSE_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Convert cepstrum to minimum phase impulse response.
 *
 * The input is the @f$M_1@f$-th order cepstral coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     c(0), & c(1), & \ldots, & c(M_1),
 *   \end{array}
 * @f]
 * and the output is the @f$M_2@f$-th order impulse response:
 * @f[
 *   \begin{array}{cccc}
 *     h(0), & h(1), & \ldots, & h(M_2).
 *   \end{array}
 * @f]
 * The truncated impulse response is obtained from the following recursion
 * formula:
 * @f[
 *   h(n) = \left\{ \begin{array}{ll}
 *     \exp c(0), & n = 0 \\
 *     \displaystyle\sum_{k=1}^{n} \frac{k}{n} c(k) h(n-k). & n > 0
 *   \end{array} \right.
 * @f]
 *
 * [1] A. V. Oppenheim and R. W. Schafer, &quot;Discrete-time signal
 *     processing, 3rd edition,&quot; Prentice-Hall Signal Processing Series,
 *     pp. 985-986, 2009.
 */
class CepstrumToMinimumPhaseImpulseResponse {
 public:
  /**
   * @param[in] num_input_order Order of cepstral coefficients, @f$M_1@f$.
   * @param[in] num_output_order Order of impulse response, @f$M_2@f$.
   */
  CepstrumToMinimumPhaseImpulseResponse(int num_input_order,
                                        int num_output_order);

  virtual ~CepstrumToMinimumPhaseImpulseResponse() {
  }

  /**
   * @return Order of cepstral coefficients.
   */
  int GetNumInputOrder() const {
    return num_input_order_;
  }

  /**
   * @return Order of impulse response.
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
   * @param[in] cepstrum @f$M_1@f$-th order cepstral coefficients.
   * @param[out] minimum_phase_impulse_response @f$M_2@f$-th order impulse
   *             response.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& cepstrum,
           std::vector<double>* minimum_phase_impulse_response) const;

 private:
  const int num_input_order_;
  const int num_output_order_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(CepstrumToMinimumPhaseImpulseResponse);
};

}  // namespace sptk

#endif  // SPTK_CONVERSION_CEPSTRUM_TO_MINIMUM_PHASE_IMPULSE_RESPONSE_H_
