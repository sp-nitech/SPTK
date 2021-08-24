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

#ifndef SPTK_ANALYSIS_ADAPTIVE_MEL_GENERALIZED_CEPSTRAL_ANALYSIS_H_
#define SPTK_ANALYSIS_ADAPTIVE_MEL_GENERALIZED_CEPSTRAL_ANALYSIS_H_

#include <vector>  // std::vector

#include "SPTK/analysis/adaptive_generalized_cepstral_analysis.h"
#include "SPTK/analysis/adaptive_mel_cepstral_analysis.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * This is a wrapper of AdaptiveGeneralizedCepstralAnalysis and
 * AdaptiveMelCepstralAnalysis.
 */
class AdaptiveMelGeneralizedCepstralAnalysis {
 public:
  /**
   * Buffer for AdaptiveMelGeneralizedCepstralAnalysis class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    AdaptiveGeneralizedCepstralAnalysis::Buffer
        buffer_for_generalized_cepstral_analysis_;
    AdaptiveMelCepstralAnalysis::Buffer buffer_for_mel_cepstral_analysis_;

    friend class AdaptiveMelGeneralizedCepstralAnalysis;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_order Order of mel-cepstral coefficients, @f$M@f$.
   * @param[in] num_pade_order Order of Pade approximation.
   * @param[in] num_stage Number of stages, @f$C@f$.
   * @param[in] alpha Frequency warping factor, @f$\alpha@f$.
   * @param[in] min_epsilon Minimum value of @f$\epsilon@f$.
   * @param[in] momentum Momentum, @f$\tau@f$.
   * @param[in] forgetting_factor Forgetting factor, @f$\lambda@f$.
   * @param[in] step_size_factor Step-size factor, @f$a@f$.
   */
  AdaptiveMelGeneralizedCepstralAnalysis(int num_order, int num_pade_order,
                                         int num_stage, double alpha,
                                         double min_epsilon, double momentum,
                                         double forgetting_factor,
                                         double step_size_factor);

  virtual ~AdaptiveMelGeneralizedCepstralAnalysis() {
  }

  /**
   * @return Order of mel-generalized cepstral coefficients.
   */
  int GetNumOrder() const {
    return mel_cepstral_analysis_.GetNumOrder();
  }

  /**
   * @return Order of Pade approximation.
   */
  int GetNumPadeOrder() const {
    return mel_cepstral_analysis_.GetNumPadeOrder();
  }

  /**
   * @return Number of stages.
   */
  int GetNumStage() const {
    return generalized_cepstral_analysis_.GetNumStage();
  }

  /**
   * @return Frequency warping factor.
   */
  double GetAlpha() const {
    return mel_cepstral_analysis_.GetAlpha();
  }

  /**
   * @return Minimum epsilon.
   */
  double GetMinEpsilon() const {
    return mel_cepstral_analysis_.GetMinEpsilon();
  }

  /**
   * @return Momentum.
   */
  double GetMomentum() const {
    return mel_cepstral_analysis_.GetMomentum();
  }

  /**
   * @return Forgetting factor.
   */
  double GetForgettingFactor() const {
    return mel_cepstral_analysis_.GetForgettingFactor();
  }

  /**
   * @return Step-size factor.
   */
  double GetStepSizeFactor() const {
    return mel_cepstral_analysis_.GetStepSizeFactor();
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
   * @param[out] mel_generalized_cepstrum @f$M@f$-th order mel-generalized
   *             cepstral coefficients.
   * @param[in,out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(double input_signal, double* prediction_error,
           std::vector<double>* mel_generalized_cepstrum,
           AdaptiveMelGeneralizedCepstralAnalysis::Buffer* buffer) const;

 private:
  const AdaptiveGeneralizedCepstralAnalysis generalized_cepstral_analysis_;
  const AdaptiveMelCepstralAnalysis mel_cepstral_analysis_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(AdaptiveMelGeneralizedCepstralAnalysis);
};

}  // namespace sptk

#endif  // SPTK_ANALYSIS_ADAPTIVE_MEL_GENERALIZED_CEPSTRAL_ANALYSIS_H_
