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

#ifndef SPTK_ANALYZER_MEL_CEPSTRAL_ANALYSIS_H_
#define SPTK_ANALYZER_MEL_CEPSTRAL_ANALYSIS_H_

#include <vector>  // std::vector

#include "SPTK/math/frequency_transform.h"
#include "SPTK/math/real_valued_fast_fourier_transform.h"
#include "SPTK/math/real_valued_inverse_fast_fourier_transform.h"
#include "SPTK/math/toeplitz_plus_hankel_system_solver.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

class MelCepstralAnalysis {
 public:
  class Buffer {
   public:
    Buffer() {
    }
    virtual ~Buffer() {
    }

   private:
    std::vector<double> log_periodogram_;
    std::vector<double> cepstrum_;
    std::vector<double> imaginary_part_output_;
    std::vector<double> b_;
    std::vector<double> d_;
    std::vector<double> r_;
    std::vector<double> rt_;
    std::vector<double> rr_;
    std::vector<double> ra_;
    std::vector<double> gradient_;

    RealValuedFastFourierTransform::Buffer buffer_for_fourier_transform_;
    RealValuedInverseFastFourierTransform::Buffer
        buffer_for_inverse_fourier_transform_;
    FrequencyTransform::Buffer buffer_for_frequency_transform_;
    FrequencyTransform::Buffer buffer_for_inverse_frequency_transform_;
    ToeplitzPlusHankelSystemSolver::Buffer buffer_for_system_solver_;

    friend class MelCepstralAnalysis;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  //
  MelCepstralAnalysis(int fft_length, int num_order, double alpha,
                      int num_iteration, double convergence_threshold);

  //
  virtual ~MelCepstralAnalysis() {
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
           std::vector<double>* mel_cepstrum,
           MelCepstralAnalysis::Buffer* buffer) const;

 private:
  //
  const int fft_length_;

  //
  const int num_order_;

  //
  const double alpha_;

  //
  const int num_iteration_;

  //
  const double convergence_threshold_;

  //
  const RealValuedFastFourierTransform fourier_transform_;

  //
  const RealValuedInverseFastFourierTransform inverse_fourier_transform_;

  //
  const FrequencyTransform frequency_transform_;

  //
  const FrequencyTransform inverse_frequency_transform_;

  //
  const ToeplitzPlusHankelSystemSolver toeplitz_plus_hankel_system_solver_;

  //
  bool is_valid_;

  //
  std::vector<double> alpha_vector_;

  //
  DISALLOW_COPY_AND_ASSIGN(MelCepstralAnalysis);
};

}  // namespace sptk

#endif  // SPTK_ANALYZER_MEL_CEPSTRAL_ANALYSIS_H_
