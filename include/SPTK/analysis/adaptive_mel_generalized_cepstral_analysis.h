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

#ifndef SPTK_ANALYSIS_ADAPTIVE_MEL_GENERALIZED_CEPSTRAL_ANALYSIS_H_
#define SPTK_ANALYSIS_ADAPTIVE_MEL_GENERALIZED_CEPSTRAL_ANALYSIS_H_

#include <vector>  // std::vector

#include "SPTK/analysis/adaptive_generalized_cepstral_analysis.h"
#include "SPTK/analysis/adaptive_mel_cepstral_analysis.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

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
