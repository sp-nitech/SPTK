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
//                1996-2019  Nagoya Institute of Technology          //
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

#ifndef SPTK_ANALYZER_MEL_GENERALIZED_CEPSTRAL_ANALYSIS_H_
#define SPTK_ANALYZER_MEL_GENERALIZED_CEPSTRAL_ANALYSIS_H_

#include <vector>  // std::vector

#include "SPTK/analyzer/mel_cepstral_analysis.h"
#include "SPTK/converter/mel_cepstrum_to_mlsa_digital_filter_coefficients.h"
#include "SPTK/converter/mel_generalized_cepstrum_to_mel_generalized_cepstrum.h"
#include "SPTK/converter/mlsa_digital_filter_coefficients_to_mel_cepstrum.h"
#include "SPTK/math/fast_fourier_transform_for_real_sequence.h"
#include "SPTK/math/inverse_fast_fourier_transform.h"
#include "SPTK/math/inverse_fast_fourier_transform_for_real_sequence.h"
#include "SPTK/math/toeplitz_plus_hankel_system_solver.h"
#include "SPTK/normalizer/generalized_cepstrum_gain_normalization.h"
#include "SPTK/normalizer/generalized_cepstrum_inverse_gain_normalization.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

class MelGeneralizedCepstralAnalysis {
 public:
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

    FastFourierTransformForRealSequence::Buffer buffer_for_fourier_transform_;
    InverseFastFourierTransformForRealSequence::Buffer
        buffer_for_inverse_fourier_transform_;
    ToeplitzPlusHankelSystemSolver::Buffer buffer_for_system_solver_;
    MelGeneralizedCepstrumToMelGeneralizedCepstrum::Buffer
        buffer_for_mel_generalized_cepstrum_transform_;
    MelCepstralAnalysis::Buffer buffer_for_mel_cepstral_analysis_;

    friend class MelGeneralizedCepstralAnalysis;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  //
  MelGeneralizedCepstralAnalysis(int fft_length, int num_order, double alpha,
                                 double gamma, int num_iteration,
                                 double convergence_threshold);

  //
  virtual ~MelGeneralizedCepstralAnalysis() {
    if (mel_cepstral_analysis_) delete mel_cepstral_analysis_;
  }

  //
  int GetFftLength() const {
    return fft_length_;
  }

  //
  int GetNumOrder() const {
    return num_order_;
  }

  //
  double GetAlpha() const {
    return alpha_;
  }

  //
  double GetGamma() const {
    return gamma_;
  }

  //
  int GetNumIteration() const {
    return num_iteration_;
  }

  //
  double GetConvergenceThreshold() const {
    return convergence_threshold_;
  }

  //
  bool IsValid() const {
    return is_valid_;
  }

  //
  bool Run(const std::vector<double>& periodogram,
           std::vector<double>* mel_generalized_cepstrum,
           MelGeneralizedCepstralAnalysis::Buffer* buffer) const;

 private:
  //
  bool NewtonRaphsonMethod(
      double gamma, double* epsilon,
      MelGeneralizedCepstralAnalysis::Buffer* buffer) const;

  //
  const int fft_length_;

  //
  const int num_order_;

  //
  const double alpha_;

  //
  const double gamma_;

  //
  const int num_iteration_;

  //
  const double convergence_threshold_;

  //
  const FastFourierTransformForRealSequence fourier_transform_;

  //
  const InverseFastFourierTransformForRealSequence inverse_fourier_transform_;

  //
  const InverseFastFourierTransform
      inverse_fourier_transform_for_complex_sequence_;

  //
  const ToeplitzPlusHankelSystemSolver toeplitz_plus_hankel_system_solver_;

  //
  const GeneralizedCepstrumGainNormalization
      generalized_cepstrum_gain_normalization_;

  //
  const GeneralizedCepstrumInverseGainNormalization
      generalized_cepstrum_inverse_gain_normalization_gamma_minus_one_;

  //
  const GeneralizedCepstrumInverseGainNormalization
      generalized_cepstrum_inverse_gain_normalization_;

  //
  const MelCepstrumToMlsaDigitalFilterCoefficients
      mel_cepstrum_to_mlsa_digital_filter_coefficients_;

  //
  const MlsaDigitalFilterCoefficientsToMelCepstrum
      mlsa_digital_filter_coefficients_to_mel_cepstrum_;

  //
  const MelGeneralizedCepstrumToMelGeneralizedCepstrum
      mel_generalized_cepstrum_transform_;

  //
  MelCepstralAnalysis* mel_cepstral_analysis_;

  //
  bool is_valid_;

  //
  DISALLOW_COPY_AND_ASSIGN(MelGeneralizedCepstralAnalysis);
};

}  // namespace sptk

#endif  // SPTK_ANALYZER_MEL_GENERALIZED_CEPSTRAL_ANALYSIS_H_
