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

#ifndef SPTK_CHECK_MLSA_DIGITAL_FILTER_STABILITY_CHECK_H_
#define SPTK_CHECK_MLSA_DIGITAL_FILTER_STABILITY_CHECK_H_

#include <vector>  // std::vector

#include "SPTK/math/inverse_fast_fourier_transform.h"
#include "SPTK/math/real_valued_fast_fourier_transform.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Check stability of mel-cepstral coefficients and modify them.
 *
 * The input is the @f$M@f$-th order mel-cepstrum:
 * @f[
 *   \begin{array}{cccc}
 *     K, & \tilde{c}(1), & \ldots, & \tilde{c}(M),
 *   \end{array}
 * @f]
 * and the output is the modified @f$M@f$-th order mel-cepstrum:
 * @f[
 *   \begin{array}{cccc}
 *     K, & \tilde{c}'(1), & \ldots, & \tilde{c}'(M).
 *   \end{array}
 * @f]
 *
 * In the mel-cepstral analysis, spectral envelope is modeled by @f$M@f$-th
 * order mel-cepstral coefficients:
 * @f[
 *   H(z) = \exp \sum_{m=0}^M \tilde{c}(m) \tilde{z}^{-m}
 * @f]
 * where
 * @f[
 *   \tilde{z}^{-1} = \frac{z^{-1} - \alpha}{1 - \alpha z^{-1}}.
 * @f]
 * The @f$H(z)@f$ can be decomposed as @f$K \cdot D(z)@f$ where
 * @f{eqnarray}{
 *      K &=& \exp \, b(0), \\
 *   D(z) &=& \exp \sum_{m=1}^M b(m) \varPhi_m(z),
 * @f}
 * and
 * @f[
 *   \varPhi_m(z) = \left\{ \begin{array}{ll}
 *     1, & m = 0 \\
 *     \displaystyle\frac{(1 - \alpha^2)z^{-1}}{1 - \alpha z^{-1}}
 *         \tilde{z}^{-(m - 1)}. & m > 0
 *   \end{array} \right.
 * @f]
 * The exponential transfer function @f$D(z)@f$ is implemented by an @f$L@f$-th
 * order rational function @f$R_L(\cdot)@f$ using the modified Pade
 * approximation:
 * @f[
 *   D(z) \equiv \exp F(z) \simeq R_L(F(z))
 * @f]
 * where
 * @f[
 *   F(z) = \sum_{m=1}^M b(m) \varPhi_m(z).
 * @f]
 * The stability of the MLSA digital filter can be checked by the maximum
 * magnitude of the basic filter @f$F(z)@f$. It can be simply obtained by
 * applying the fast Fourier transform to the gain normalized mel-cepstrum
 * sequence. In addition, by assuming that the amplitude spectrum of human
 * speech at zero frequency usually takes maximum value, we can check the
 * stability without FFT.
 */
class MlsaDigitalFilterStabilityCheck {
 public:
  /**
   * Type of modification.
   */
  enum ModificationType { kClipping = 0, kScaling, kNumModificationTypes };

  /**
   * Buffer of MlsaDigitalFilterStabilityCheck.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    std::vector<double> amplitude_;
    std::vector<double> fourier_transform_real_part_;
    std::vector<double> fourier_transform_imag_part_;
    RealValuedFastFourierTransform::Buffer fourier_transform_buffer_;

    friend class MlsaDigitalFilterStabilityCheck;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_order Order of mel-cepstrum, @f$M@f$.
   * @param[in] alpha All-pass constant, @f$\alpha@f$.
   * @param[in] threshold Threshold value.
   */
  MlsaDigitalFilterStabilityCheck(int num_order, double alpha,
                                  double threshold);

  /**
   * @param[in] num_order Order of mel-cepstrum, @f$M@f$.
   * @param[in] alpha All-pass constant, @f$\alpha@f$.
   * @param[in] threshold Threshold value.
   * @param[in] fft_length FFT length (valid if fast_mode is true).
   * @param[in] modification_type Type of modification.
   */
  MlsaDigitalFilterStabilityCheck(int num_order, double alpha, double threshold,
                                  int fft_length,
                                  ModificationType modification_type);

  virtual ~MlsaDigitalFilterStabilityCheck();

  /**
   * @return Order of coefficients.
   */
  int GetNumOrder() const {
    return num_order_;
  }

  /**
   * @return All-pass constant.
   */
  double GetAlpha() const {
    return alpha_;
  }

  /**
   * @return Threshold value.
   */
  double GetThreshold() const {
    return threshold_;
  }

  /**
   * @return True if fast mode is on.
   */
  bool GetFastModeFlag() const {
    return fast_mode_;
  }

  /**
   * @return FFT length.
   */
  int GetFftLength() const {
    return fft_length_;
  }

  /**
   * @return Type of modification.
   */
  ModificationType GetModificationType() const {
    return modification_type_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] mel_cepstrum @f$M@f$-th order mel-cepstrum.
   * @param[out] modified_mel_cepstrum Modified @f$M@f$-th order mel-cepstrum
   *             (optional).
   * @param[out] is_stable True if the given coefficients are stable.
   * @param[out] maximum_amplitude_of_basic_filter Maximum amplitude (optional).
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& mel_cepstrum,
           std::vector<double>* modified_mel_cepstrum, bool* is_stable,
           double* maximum_amplitude_of_basic_filter,
           MlsaDigitalFilterStabilityCheck::Buffer* buffer) const;

  /**
   * @param[in,out] input_and_output @f$M@f$-th order coefficients.
   * @param[out] is_stable True if the given coefficients are stable.
   * @param[out] maximum_amplitude_of_basic_filter Maximum amplitude (optional).
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(std::vector<double>* input_and_output, bool* is_stable,
           double* maximum_amplitude_of_basic_filter,
           MlsaDigitalFilterStabilityCheck::Buffer* buffer) const;

 private:
  const int num_order_;
  const double alpha_;
  const double threshold_;
  const bool fast_mode_;
  const int fft_length_;
  const ModificationType modification_type_;

  RealValuedFastFourierTransform* fourier_transform_;
  InverseFastFourierTransform* inverse_fourier_transform_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(MlsaDigitalFilterStabilityCheck);
};

}  // namespace sptk

#endif  // SPTK_CHECK_MLSA_DIGITAL_FILTER_STABILITY_CHECK_H_
