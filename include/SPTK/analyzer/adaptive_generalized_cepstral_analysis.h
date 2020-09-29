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

#ifndef SPTK_ANALYZER_ADAPTIVE_GENERALIZED_CEPSTRAL_ANALYSIS_H_
#define SPTK_ANALYZER_ADAPTIVE_GENERALIZED_CEPSTRAL_ANALYSIS_H_

#include <vector>  // std::vector

#include "SPTK/converter/generalized_cepstrum_inverse_gain_normalization.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Perform adaptive generalized cesptral analysis.
 *
 * Let assume \f$x(n)\f$ is an input signal and \f$e(n)\f$ is the output of the
 * inverse filter \f$(1+F(z))^{-\frac{1}{\gamma}}\f$. The relationship between
 * \f$x(n)\f$ and \f$e(n)\f$ is shown as below:
 * @image html agcep_1.png
 *
 * The filter \f$F(z)\f$ is represented as
 * \f[
 *   F(z) = \sum_{m=1}^M c'_\gamma(m) z^{-m},
 * \f]
 * where \f$c'_\gamma(m)\f$ is the normalized generalized cepstral coefficients.
 * The coefficients of the filter \f$F(z)\f$ is updated every sample as
 * \f[
 *   \boldsymbol{c}'^{(n+1)}_\gamma =
 *     \boldsymbol{c}'^{(n)}_\gamma - \mu^{(n)} \bar{\nabla} \epsilon^{(n)}
 * \f]
 * where
 * \f{eqnarray}{
 *   \mu^{(n)} &=& \frac{a}{M \epsilon^{(n)}}, \\
 *   \epsilon^{(n)} &=& \lambda \epsilon^{(n-1)} + (1-\lambda) e_\gamma^2(n),
 * \f}
 * and \f$a\f$ is the step-size factor and \f$\lambda\f$ is the forgetting
 * factor. If \f$\epsilon^{(n)}\f$ is less than \f$\epsilon_{min}\f$,
 * \f$\epsilon^{(n)}\f$ is set to \f$\epsilon_{min}\f$. The estimate of
 * \f$\nabla \epsilon\f$ is
 * \f[
 *   \bar{\nabla} \epsilon^{(n)} = \tau \bar{\nabla} \epsilon^{(n-1)}
 *     -2 (1-\tau) e_\gamma(n) \boldsymbol{e}^{(n)}_{\gamma}
 * \f]
 * where \f$\tau\f$ is the moment and
 * \f$\boldsymbol{e}^{(n)}_{\gamma} =
 *   [e_\gamma(n-1),\ldots,e_\gamma(n-M)]^{\mathsf{T}}\f$
 * is the set of outputs of the filter
 *   \f$(1+\gamma F(z))^{-\frac{1}{\gamma}-1}\f$.
 *
 * The coefficients of the filter \f$F(z)\f$ are denormalized to obtain the
 * generalized cepstral coefficients.
 *
 * @sa sptk::GeneralizedCepstrumInverseGainNormalization
 */
class AdaptiveGeneralizedCepstralAnalysis {
 public:
  /**
   * Buffer for AdaptiveGeneralizedCepstralAnalysis class.
   */
  class Buffer {
   public:
    Buffer() : prev_adjusted_error_(1.0), prev_epsilon_(1.0) {
    }

    virtual ~Buffer() {
    }

   private:
    double prev_adjusted_error_;
    double prev_epsilon_;

    std::vector<double> normalized_generalized_cepstrum_;
    std::vector<double> d_;
    std::vector<double> gradient_;

    friend class AdaptiveGeneralizedCepstralAnalysis;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_order Order of cepstral coefficients, \f$M\f$.
   * @param[in] num_stage Number of stages, \f$C\f$.
   * @param[in] min_epsilon Minimum value of \f$\epsilon\f$.
   * @param[in] momentum Momentum, \f$\tau\f$.
   * @param[in] forgetting_factor Forgetting factor, \f$\lambda\f$.
   * @param[in] step_size_factor Step-size factor, \f$a\f$.
   */
  AdaptiveGeneralizedCepstralAnalysis(int num_order, int num_stage,
                                      double min_epsilon, double momentum,
                                      double forgetting_factor,
                                      double step_size_factor);

  virtual ~AdaptiveGeneralizedCepstralAnalysis() {
  }

  /**
   * @return Order of cepstral coefficients.
   */
  int GetNumOrder() const {
    return generalized_cepstrum_inverse_gain_normalization_.GetNumOrder();
  }

  /**
   * @return Number of stages.
   */
  int GetNumStage() const {
    return num_stage_;
  }

  /**
   * @return Gamma.
   */
  double GetGamma() const {
    return generalized_cepstrum_inverse_gain_normalization_.GetGamma();
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
   * @return True if this obejct is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] input_signal An input signal, \f$x(n)\f$.
   * @param[out] prediction_error A prediction error, \f$e(n)\f$.
   * @param[out] generalized_cepstrum \f$M\f$-th order generalized cepstral
   *             coefficients.
   * @param[in,out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(double input_signal, double* prediction_error,
           std::vector<double>* generalized_cepstrum,
           AdaptiveGeneralizedCepstralAnalysis::Buffer* buffer) const;

 private:
  const int num_stage_;
  const double min_epsilon_;
  const double momentum_;
  const double forgetting_factor_;
  const double step_size_factor_;

  const GeneralizedCepstrumInverseGainNormalization
      generalized_cepstrum_inverse_gain_normalization_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(AdaptiveGeneralizedCepstralAnalysis);
};

}  // namespace sptk

#endif  // SPTK_ANALYZER_ADAPTIVE_GENERALIZED_CEPSTRAL_ANALYSIS_H_
