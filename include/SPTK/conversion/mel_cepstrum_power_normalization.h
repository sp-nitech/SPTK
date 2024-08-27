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

#ifndef SPTK_CONVERSION_MEL_CEPSTRUM_POWER_NORMALIZATION_H_
#define SPTK_CONVERSION_MEL_CEPSTRUM_POWER_NORMALIZATION_H_

#include <vector>  // std::vector

#include "SPTK/conversion/cepstrum_to_autocorrelation.h"
#include "SPTK/math/frequency_transform.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Convert mel-cepstral coefficients to power-normalized ones.
 *
 * The input is the @f$M@f$-th order mel-cepstral coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     \tilde{c}(0), & \tilde{c}(1), & \ldots, & \tilde{c}(M),
 *   \end{array}
 * @f]
 * and the output are the @f$M@f$-th order power-normalized ones:
 * @f[
 *   \begin{array}{ccccc}
 *     \tilde{c}'(0), & \tilde{c}'(1), & \ldots, & \tilde{c}'(M),
 *   \end{array}
 * @f]
 * and @f$\log P@f$ where @f$P@f$ is the power and
 * @f[
 *   \tilde{c}'(m) = \left\{ \begin{array}{ll}
 *     \tilde{c}(0) - \log \sqrt{P}, & m = 0 \\
 *     \tilde{c}(m). & 1 \le m \le M
 *   \end{array} \right.
 * @f]
 */
class MelCepstrumPowerNormalization {
 public:
  /**
   * Buffer for MelCepstrumPowerNormalization class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    std::vector<double> cepstrum_;
    std::vector<double> autocorrelation_;
    FrequencyTransform::Buffer buffer_for_frequency_transform_;
    CepstrumToAutocorrelation::Buffer buffer_for_cepstrum_to_autocorrelation_;

    friend class MelCepstrumPowerNormalization;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_order Order of coefficients, @f$M@f$.
   * @param[in] impulse_response_length Length of impuse response.
   * @param[in] alpha All-pass constant, @f$\alpha@f$.
   */
  MelCepstrumPowerNormalization(int num_order, int impulse_response_length,
                                double alpha);

  virtual ~MelCepstrumPowerNormalization() {
  }

  /**
   * @return Order of coefficients.
   */
  int GetNumOrder() const {
    return frequency_transform_.GetNumInputOrder();
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] mel_cepstrum @f$M@f$-th order mel-cepstral coefficients.
   * @param[out] power_normalized_mel_cepstrum @f$M@f$-th order
   *             power-normalized mel-cepstral coefficients.
   * @param[out] power Logarithm of power.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& mel_cepstrum,
           std::vector<double>* power_normalized_mel_cepstrum, double* power,
           MelCepstrumPowerNormalization::Buffer* buffer) const;

  /**
   * @param[in,out] input_and_output @f$M@f$-th order coefficients.
   * @param[out] power Logarithm of power.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(std::vector<double>* input_and_output, double* power,
           MelCepstrumPowerNormalization::Buffer* buffer) const;

 private:
  const FrequencyTransform frequency_transform_;
  const CepstrumToAutocorrelation cepstrum_to_autocorrelation_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(MelCepstrumPowerNormalization);
};

}  // namespace sptk

#endif  // SPTK_CONVERSION_MEL_CEPSTRUM_POWER_NORMALIZATION_H_
