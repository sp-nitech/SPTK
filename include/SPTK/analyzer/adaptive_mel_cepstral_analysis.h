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
//                1996-2018  Nagoya Institute of Technology          //
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

#ifndef SPTK_ANALYZER_ADAPTIVE_MEL_CEPSTRAL_ANALYSIS_H_
#define SPTK_ANALYZER_ADAPTIVE_MEL_CEPSTRAL_ANALYSIS_H_

#include <vector>  // std::vector

#include "SPTK/converter/mlsa_digital_filter_coefficients_to_mel_cepstrum.h"
#include "SPTK/filter/mlsa_digital_filter.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

class AdaptiveMelCepstralAnalysis {
 public:
  class Buffer {
   public:
    Buffer() : prev_prediction_error_(0.0), prev_epsilon_(1.0) {
    }
    virtual ~Buffer() {
    }

   private:
    std::vector<double> mlsa_digital_filter_coefficients_;
    std::vector<double> inverse_mlsa_digital_filter_coefficients_;
    std::vector<double> gradient_;
    std::vector<double> buffer_for_phi_digital_filter_;
    MlsaDigitalFilter::Buffer buffer_for_mlsa_digital_filter_;
    double prev_prediction_error_;
    double prev_epsilon_;
    friend class AdaptiveMelCepstralAnalysis;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  //
  AdaptiveMelCepstralAnalysis(int num_order, int num_pade_order, double alpha,
                              double minimum_epsilon, double momentum,
                              double forgetting_factor,
                              double step_size_factor);

  //
  virtual ~AdaptiveMelCepstralAnalysis() {
  }

  //
  int GetNumOrder() const {
    return mlsa_digital_filter_.GetNumFilterOrder();
  }

  //
  int GetNumPadeOrder() const {
    return mlsa_digital_filter_.GetNumPadeOrder();
  }

  //
  double GetAlpha() const {
    return mlsa_digital_filter_.GetAlpha();
  }

  //
  double GetMinimumEpsilon() const {
    return minimum_epsilon_;
  }

  //
  double GetMomentum() const {
    return momentum_;
  }

  //
  double GetForgettingFactor() const {
    return forgetting_factor_;
  }

  //
  double GetStepSizeFactor() const {
    return step_size_factor_;
  }

  //
  bool IsValid() const {
    return is_valid_;
  }

  //
  bool Run(double input_signal, double* prediction_error,
           std::vector<double>* mel_cepstrum,
           AdaptiveMelCepstralAnalysis::Buffer* buffer) const;

 private:
  //
  const double minimum_epsilon_;

  //
  const double momentum_;

  //
  const double forgetting_factor_;

  //
  const double step_size_factor_;

  //
  const MlsaDigitalFilter mlsa_digital_filter_;

  //
  const MlsaDigitalFilterCoefficientsToMelCepstrum
      mlsa_digital_filter_coefficients_to_mel_cepstrum_;

  //
  bool is_valid_;

  //
  DISALLOW_COPY_AND_ASSIGN(AdaptiveMelCepstralAnalysis);
};

}  // namespace sptk

#endif  // SPTK_ANALYZER_ADAPTIVE_MEL_CEPSTRAL_ANALYSIS_H_
