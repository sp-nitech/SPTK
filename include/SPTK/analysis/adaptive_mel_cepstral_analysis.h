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

#ifndef SPTK_ANALYSIS_ADAPTIVE_MEL_CEPSTRAL_ANALYSIS_H_
#define SPTK_ANALYSIS_ADAPTIVE_MEL_CEPSTRAL_ANALYSIS_H_

#include <vector>  // std::vector

#include "SPTK/conversion/mlsa_digital_filter_coefficients_to_mel_cepstrum.h"
#include "SPTK/filter/mlsa_digital_filter.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Perform adaptive mel-cesptral analysis.
 *
 * The block diagram of the adaptive mel-cepstral analysis is shown as below:
 * @image html amcep_1.png
 *
 * where @f$x(n)@f$ is an input signal and @f$e(n)@f$ is the output of the
 * inverse filter @f$1/D(z)@f$. The @f$D(z)@f$ is implemented as a MLSA filter.
 * The coefficients of the MLSA filter @f$\boldsymbol{b}@f$ is updated every
 * sample as
 * @f[
 *   \boldsymbol{b}^{(n+1)} =
 *     \boldsymbol{b}^{(n)} - \mu^{(n)} \bar{\nabla} \epsilon^{(n)}
 * @f]
 * where
 * @f{eqnarray}{
 *   \mu^{(n)} &=& \frac{a}{M \epsilon^{(n)}}, \\
 *   \epsilon^{(n)} &=& \lambda \epsilon^{(n-1)} + (1-\lambda) e^2(n),
 * @f}
 * and @f$a@f$ is the step-size factor and @f$\lambda@f$ is the forgetting
 * factor. If @f$\epsilon^{(n)}@f$ is less than @f$\epsilon_{min}@f$,
 * @f$\epsilon^{(n)}@f$ is set to @f$\epsilon_{min}@f$. The estimate of
 * @f$\nabla \epsilon@f$ is
 * @f[
 *   \bar{\nabla} \epsilon^{(n)} = \tau \bar{\nabla} \epsilon^{(n-1)}
 *     -2 (1-\tau) e(n) \boldsymbol{e}^{(n)}_{\Phi}
 * @f]
 * where @f$\tau@f$ is the moment and
 * @f$\boldsymbol{e}^{(n)}_{\Phi}=[e_1(n),e_2(n),\ldots,e_M(n)]^{\mathsf{T}}@f$
 * is the set of outputs of the filter @f$\Phi_m(z)@f$:
 * @image html amcep_2.png
 *
 * The coefficients of the MLSA filter are converted to the mel-cepstral
 * coefficients by a linear transformation.
 *
 * @sa sptk::MlsaDigitalFilterCoefficientsToMelCepstrum
 */
class AdaptiveMelCepstralAnalysis {
 public:
  /**
   * Buffer for AdaptiveMelCepstralAnalysis class.
   */
  class Buffer {
   public:
    Buffer() : prev_prediction_error_(0.0), prev_epsilon_(1.0) {
    }

    virtual ~Buffer() {
    }

   private:
    double prev_prediction_error_;
    double prev_epsilon_;

    std::vector<double> mlsa_digital_filter_coefficients_;
    std::vector<double> inverse_mlsa_digital_filter_coefficients_;
    std::vector<double> buffer_for_phi_digital_filter_;
    std::vector<double> gradient_;
    MlsaDigitalFilter::Buffer buffer_for_mlsa_digital_filter_;

    friend class AdaptiveMelCepstralAnalysis;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_order Order of mel-cepstral coefficients, @f$M@f$.
   * @param[in] num_pade_order Order of Pade approximation.
   * @param[in] alpha Frequency warping factor, @f$\alpha@f$.
   * @param[in] min_epsilon Minimum value of @f$\epsilon@f$.
   * @param[in] momentum Momentum, @f$\tau@f$.
   * @param[in] forgetting_factor Forgetting factor, @f$\lambda@f$.
   * @param[in] step_size_factor Step-size factor, @f$a@f$.
   */
  AdaptiveMelCepstralAnalysis(int num_order, int num_pade_order, double alpha,
                              double min_epsilon, double momentum,
                              double forgetting_factor,
                              double step_size_factor);

  virtual ~AdaptiveMelCepstralAnalysis() {
  }

  /**
   * @return Order of mel-cepstral coefficients.
   */
  int GetNumOrder() const {
    return mlsa_digital_filter_.GetNumFilterOrder();
  }

  /**
   * @return Order of Pade approximation.
   */
  int GetNumPadeOrder() const {
    return mlsa_digital_filter_.GetNumPadeOrder();
  }

  /**
   * @return Frequency warping factor.
   */
  double GetAlpha() const {
    return mlsa_digital_filter_.GetAlpha();
  }

  /**
   * @return Minimum epsilon.
   */
  double GetMinEpsilon() const {
    return min_epsilon_;
  }

  /**
   * @return Momentum.
   */
  double GetMomentum() const {
    return momentum_;
  }

  /**
   * @return Forgetting factor.
   */
  double GetForgettingFactor() const {
    return forgetting_factor_;
  }

  /**
   * @return Step-size factor.
   */
  double GetStepSizeFactor() const {
    return step_size_factor_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] input_signal An input signal, @f$x(n)@f$.
   * @param[out] prediction_error A prediction error, @f$e(n)@f$.
   * @param[out] mel_cepstrum @f$M@f$-th order mel-cepstral coefficients.
   * @param[in,out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(double input_signal, double* prediction_error,
           std::vector<double>* mel_cepstrum,
           AdaptiveMelCepstralAnalysis::Buffer* buffer) const;

 private:
  const double min_epsilon_;
  const double momentum_;
  const double forgetting_factor_;
  const double step_size_factor_;

  const MlsaDigitalFilter mlsa_digital_filter_;
  const MlsaDigitalFilterCoefficientsToMelCepstrum
      mlsa_digital_filter_coefficients_to_mel_cepstrum_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(AdaptiveMelCepstralAnalysis);
};

}  // namespace sptk

#endif  // SPTK_ANALYSIS_ADAPTIVE_MEL_CEPSTRAL_ANALYSIS_H_
