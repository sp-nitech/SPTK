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

#ifndef SPTK_CONVERSION_MEL_CEPSTRUM_INVERSE_POWER_NORMALIZATION_H_
#define SPTK_CONVERSION_MEL_CEPSTRUM_INVERSE_POWER_NORMALIZATION_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Convert power-normalized mel-cepstral coefficients to mel-cepstral ones.
 *
 * The input are the @f$M@f$-th order power-normalized mel-cepstral
 * coefficients:
 * @f[
 *   \begin{array}{ccccc}
 *     \tilde{c}'(0), & \tilde{c}'(1), & \ldots, & \tilde{c}'(M),
 *   \end{array}
 * @f]
 * and @f$\log P@f$ where @f$P@f$ is the power and the output is the @f$M@f$-th
 * order mel-cepstral coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     \tilde{c}(0), & \tilde{c}(1), & \ldots, & \tilde{c}(M),
 *   \end{array}
 * @f]
 * where
 * @f[
 *   \tilde{c}(m) = \left\{ \begin{array}{ll}
 *     \tilde{c}'(0) + \log \sqrt{P}, & m = 0 \\
 *     \tilde{c}'(m). & 1 \le m \le M
 *   \end{array} \right.
 * @f]
 */
class MelCepstrumInversePowerNormalization {
 public:
  /**
   * @param[in] num_order Order of coefficients, @f$M@f$.
   */
  explicit MelCepstrumInversePowerNormalization(int num_order);

  virtual ~MelCepstrumInversePowerNormalization() {
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
   * @param[in] power_normalized_mel_cepstrum @f$M@f$-th order
   *            power-normalized mel-cepstral coefficients.
   * @param[in] power Logarithm of power.
   * @param[out] mel_cepstrum @f$M@f$-th order mel-cepstral coefficients.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& power_normalized_mel_cepstrum,
           double power, std::vector<double>* mel_cepstrum) const;

  /**
   * @param[in,out] input_and_output @f$M@f$-th order coefficients.
   * @param[in] power Logarithm of power.
   * @return True on success, false on failure.
   */
  bool Run(std::vector<double>* input_and_output, double power) const;

 private:
  const int num_order_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(MelCepstrumInversePowerNormalization);
};

}  // namespace sptk

#endif  // SPTK_CONVERSION_MEL_CEPSTRUM_INVERSE_POWER_NORMALIZATION_H_
