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

#ifndef SPTK_ANALYSIS_MEL_GENERALIZED_CEPSTRAL_ANALYSIS_H_
#define SPTK_ANALYSIS_MEL_GENERALIZED_CEPSTRAL_ANALYSIS_H_

#include <vector>  // std::vector

#include "SPTK/analysis/mel_cepstral_analysis.h"
#include "SPTK/conversion/generalized_cepstrum_gain_normalization.h"
#include "SPTK/conversion/generalized_cepstrum_inverse_gain_normalization.h"
#include "SPTK/conversion/mel_cepstrum_to_mlsa_digital_filter_coefficients.h"
#include "SPTK/conversion/mel_generalized_cepstrum_to_mel_generalized_cepstrum.h"
#include "SPTK/conversion/mlsa_digital_filter_coefficients_to_mel_cepstrum.h"
#include "SPTK/math/inverse_fast_fourier_transform.h"
#include "SPTK/math/real_valued_fast_fourier_transform.h"
#include "SPTK/math/real_valued_inverse_fast_fourier_transform.h"
#include "SPTK/math/toeplitz_plus_hankel_system_solver.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Calculate mel-generalized cepstrum from periodogram.
 *
 * The input is the half of periodogram:
 * @f[
 *   \begin{array}{cccc}
 *     |X(0)|^2, & |X(1)|^2, & \ldots, & |X(N/2)|^2,
 *   \end{array}
 * @f]
 * where @f$N@f$ is the FFT length. The output is the @f$M@f$-th order
 * mel-generalized cepstral coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     \tilde{c}_\gamma(0), & \tilde{c}_\gamma(1), & \ldots, &
 *     \tilde{c}_\gamma(M).
 *   \end{array}
 * @f]
 *
 * In the mel-generalized cepstral analysis, the spectrum of speech signal is
 * modeled by @f$M@f$-th order mel-generalized cepstral coefficients as follows:
 * @f{eqnarray}{
 *   H(z) &=& s^{-1}_\gamma \left(
 *     \sum_{m=0}^M \tilde{c}_\gamma(m) \tilde{z}^{-m}
 *   \right) \\
 *   &=& \left\{ \begin{array}{ll}
 *     \left( 1 + \gamma \displaystyle\sum_{m=0}^M \tilde{c}_\gamma(m)
 *       \tilde{z}^{-m} \right)^{1/\gamma}, & -1 \le \gamma < 0 \\
 *     \exp \displaystyle\sum_{m=0}^M \tilde{c}_\gamma(m)
 *       \tilde{z}^{-m}, & \gamma = 0
 *   \end{array} \right.
 * @f}
 * where
 * @f[
 *   \tilde{z}^{-1} = \frac{z^{-1} - \alpha}{1 - \alpha z^{-1}}.
 * @f]
 */
class MelGeneralizedCepstralAnalysis {
 public:
  /**
   * Buffer for MelGeneralizedCepstralAnalysis class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    std::vector<double> b_;
    std::vector<double> c_;
    std::vector<double> real_;
    std::vector<double> imag_;
    std::vector<double> p_real_;
    std::vector<double> p_;
    std::vector<double> p_trim_;
    std::vector<double> q_real_;
    std::vector<double> q_imag_;
    std::vector<double> q_;
    std::vector<double> q_trim_;
    std::vector<double> r_real_;
    std::vector<double> r_imag_;
    std::vector<double> r_;
    std::vector<double> r_trim_;
    std::vector<double> buffer_for_frequency_transform_;
    std::vector<double> periodogram_;
    std::vector<double> gradient_;

    RealValuedFastFourierTransform::Buffer buffer_for_fourier_transform_;
    RealValuedInverseFastFourierTransform::Buffer
        buffer_for_inverse_fourier_transform_;
    ToeplitzPlusHankelSystemSolver::Buffer buffer_for_system_solver_;
    MelGeneralizedCepstrumToMelGeneralizedCepstrum::Buffer
        buffer_for_mel_generalized_cepstrum_transform_;
    MelCepstralAnalysis::Buffer buffer_for_mel_cepstral_analysis_;

    friend class MelGeneralizedCepstralAnalysis;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] fft_length Number of FFT bins, @f$N@f$.
   * @param[in] num_order Order of cepstral coefficients, @f$M@f$.
   * @param[in] alpha All-pass constant, @f$\alpha@f$.
   * @param[in] gamma Exponent parameter, @f$\gamma@f$.
   * @param[in] num_iteration Number of iterations of Newton method, @f$J@f$.
   * @param[in] convergence_threshold Convergence threshold, @f$\epsilon@f$.
   */
  MelGeneralizedCepstralAnalysis(int fft_length, int num_order, double alpha,
                                 double gamma, int num_iteration,
                                 double convergence_threshold);

  virtual ~MelGeneralizedCepstralAnalysis() {
    if (mel_cepstral_analysis_) delete mel_cepstral_analysis_;
  }

  /**
   * @return FFT length.
   */
  int GetFftLength() const {
    return fft_length_;
  }

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
   * @return Gamma.
   */
  double GetGamma() const {
    return gamma_;
  }

  /**
   * @return Number of iterations.
   */
  int GetNumIteration() const {
    return num_iteration_;
  }

  /**
   * @return Convergence threshold.
   */
  double GetConvergenceThreshold() const {
    return convergence_threshold_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] periodogram @f$(N/2+1)@f$-length periodogram.
   * @param[out] mel_generalized_cepstrum @f$M@f$-th order mel-generalized
   *             cepstral coefficients.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& periodogram,
           std::vector<double>* mel_generalized_cepstrum,
           MelGeneralizedCepstralAnalysis::Buffer* buffer) const;

 private:
  bool NewtonRaphsonMethod(
      double gamma, double* epsilon,
      MelGeneralizedCepstralAnalysis::Buffer* buffer) const;

  const int fft_length_;
  const int num_order_;
  const double alpha_;
  const double gamma_;
  const int num_iteration_;
  const double convergence_threshold_;

  const RealValuedFastFourierTransform fourier_transform_;
  const RealValuedInverseFastFourierTransform inverse_fourier_transform_;
  const InverseFastFourierTransform complex_valued_inverse_fourier_transform_;
  const ToeplitzPlusHankelSystemSolver toeplitz_plus_hankel_system_solver_;
  const GeneralizedCepstrumGainNormalization
      generalized_cepstrum_gain_normalization_;
  const GeneralizedCepstrumInverseGainNormalization
      generalized_cepstrum_inverse_gain_normalization_gamma_minus_one_;
  const GeneralizedCepstrumInverseGainNormalization
      generalized_cepstrum_inverse_gain_normalization_;
  const MelCepstrumToMlsaDigitalFilterCoefficients
      mel_cepstrum_to_mlsa_digital_filter_coefficients_;
  const MlsaDigitalFilterCoefficientsToMelCepstrum
      mlsa_digital_filter_coefficients_to_mel_cepstrum_;
  const MelGeneralizedCepstrumToMelGeneralizedCepstrum
      mel_generalized_cepstrum_transform_;

  MelCepstralAnalysis* mel_cepstral_analysis_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(MelGeneralizedCepstralAnalysis);
};

}  // namespace sptk

#endif  // SPTK_ANALYSIS_MEL_GENERALIZED_CEPSTRAL_ANALYSIS_H_
