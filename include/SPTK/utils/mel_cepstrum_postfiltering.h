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

#ifndef SPTK_UTILS_MEL_CEPSTRUM_POSTFILTERING_H_
#define SPTK_UTILS_MEL_CEPSTRUM_POSTFILTERING_H_

#include <vector>  // std::vector

#include "SPTK/conversion/cepstrum_to_autocorrelation.h"
#include "SPTK/conversion/mel_cepstrum_to_mlsa_digital_filter_coefficients.h"
#include "SPTK/conversion/mlsa_digital_filter_coefficients_to_mel_cepstrum.h"
#include "SPTK/math/frequency_transform.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Emphasize formant in mel-cepstum domain.
 *
 * The input is the @f$M@f$-th order mel-cepstral coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     \tilde{c}(0), & \tilde{c}(1), & \ldots, & \tilde{c}(M),
 *   \end{array}
 * @f]
 * and the output is the @f$M@f$-th order postfiltered ones:
 * @f[
 *   \begin{array}{cccc}
 *     \tilde{c}_\beta(0), & \tilde{c}_\beta(1), & \ldots, & \tilde{c}_\beta(M),
 *   \end{array}
 * @f]
 * where @f$\beta@f$ is the intensity of postfiltering. Note if @f$\beta=0@f$,
 * postfiltering is not performed.
 *
 * [1] T. Yoshimura, K. Tokuda, T. Masuko, and T. Kobayashi, &quot;Incorporating
 *     a mixed excitation model and postfilter into HMM-based text-to-speech
 *     synthesis,&quot; Systems and Computers in Japan, vol. 36, no. 12,
 *     pp. 43-50, 2005.
 */
class MelCepstrumPostfiltering {
 public:
  /**
   * Buffer for MelCepstrumPostfiltering class.
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
    std::vector<double> mel_cepstrum_;
    std::vector<double> mlsa_digital_filter_coefficients_;

    FrequencyTransform::Buffer buffer_for_frequency_transform_;
    CepstrumToAutocorrelation::Buffer buffer_for_cepstrum_to_autocorrelation_;

    friend class MelCepstrumPostfiltering;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_order Order of mel-cepstral coefficients, @f$M@f$.
   * @param[in] impulse_response_length Length of impulse response, @f$L@f$.
   * @param[in] onset_index This is typically set to two, i.e., 0th and 1st
   *            mel-cepstral coefficients are not emphasized.
   * @param[in] alpha All-pass constant, @f$\alpha@f$.
   * @param[in] beta Intensity of postfiltering, @f$\beta@f$.
   */
  MelCepstrumPostfiltering(int num_order, int impulse_response_length,
                           int onset_index, double alpha, double beta);

  virtual ~MelCepstrumPostfiltering() {
  }

  /**
   * @return Order of mel-cepstral coefficients.
   */
  int GetNumOrder() const {
    return frequency_transform_.GetNumInputOrder();
  }

  /**
   * @return Impulse response length.
   */
  int GetImpulseResponseLength() const {
    return cepstrum_to_autocorrelation_.GetFftLength();
  }

  /**
   * @return Onset index.
   */
  int GetOnsetIndex() const {
    return onset_index_;
  }

  /**
   * @return Alpha.
   */
  double GetAlpha() const {
    return -frequency_transform_.GetAlpha();
  }

  /**
   * @return Beta.
   */
  double GetBeta() const {
    return beta_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] mel_cepstrum @f$M@f$-th order mel-cepstral coefficients.
   * @param[out] postfiltered_mel_cepstrum @f$M@f$-th order postfiltered ones.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& mel_cepstrum,
           std::vector<double>* postfiltered_mel_cepstrum,
           MelCepstrumPostfiltering::Buffer* buffer) const;

  /**
   * @param[in,out] input_and_output @f$M@f$-th order mel-cepstral coefficients.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(std::vector<double>* input_and_output,
           MelCepstrumPostfiltering::Buffer* buffer) const;

 private:
  const int onset_index_;
  const double beta_;

  const FrequencyTransform frequency_transform_;
  const CepstrumToAutocorrelation cepstrum_to_autocorrelation_;
  const MelCepstrumToMlsaDigitalFilterCoefficients
      mel_cepstrum_to_mlsa_digital_filter_coefficients_;
  const MlsaDigitalFilterCoefficientsToMelCepstrum
      mlsa_digital_filter_coefficients_to_mel_cepstrum_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(MelCepstrumPostfiltering);
};

}  // namespace sptk

#endif  // SPTK_UTILS_MEL_CEPSTRUM_POSTFILTERING_H_
