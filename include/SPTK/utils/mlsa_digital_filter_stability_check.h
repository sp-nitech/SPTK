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

#ifndef SPTK_UTILS_MLSA_DIGITAL_FILTER_STABILITY_CHECK_H_
#define SPTK_UTILS_MLSA_DIGITAL_FILTER_STABILITY_CHECK_H_

#include <vector>  // std::vector

#include "SPTK/math/fast_fourier_transform_for_real_sequence.h"
#include "SPTK/math/inverse_fast_fourier_transform.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

class MlsaDigitalFilterStabilityCheck {
 public:
  //
  enum ModificationType { kClipping = 0, kScaling, kNumModificationTypes };

  //
  class Buffer {
   public:
    Buffer() {
    }
    virtual ~Buffer() {
    }

   private:
    std::vector<double> amplitude_;
    std::vector<double> fourier_transform_real_part_input_;
    std::vector<double> fourier_transform_imaginary_part_input_;
    std::vector<double> fourier_transform_real_part_output_;
    std::vector<double> fourier_transform_imaginary_part_output_;
    FastFourierTransformForRealSequence::Buffer fourier_transform_buffer_;
    friend class MlsaDigitalFilterStabilityCheck;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  //
  MlsaDigitalFilterStabilityCheck(int num_order, double alpha, double threshold,
                                  bool fast_mode, int fft_length,
                                  ModificationType modification_type);

  //
  virtual ~MlsaDigitalFilterStabilityCheck();

  //
  int GetNumOrder() const {
    return num_order_;
  }

  //
  double GetAlpha() const {
    return alpha_;
  }

  //
  double GetThreshold() const {
    return threshold_;
  }

  //
  bool GetFastModeFlag() const {
    return fast_mode_;
  }

  //
  int GetFftLength() const {
    return fourier_transform_ ? fourier_transform_->GetFftLength() : 0;
  }

  //
  ModificationType GetModificationType() const {
    return modification_type_;
  }

  //
  bool IsValid() const {
    return is_valid_;
  }

  // Check stability of MLSA digital filter.
  // The 2nd and 4th arguments of this function are allowed to be NULL.
  bool Run(const std::vector<double>& mel_cepstrum,
           std::vector<double>* modified_mel_cepstrum, bool* is_stable,
           double* maximum_amplitude_of_basic_filter,
           MlsaDigitalFilterStabilityCheck::Buffer* buffer) const;

 private:
  //
  const int num_order_;

  //
  const double alpha_;

  //
  const double threshold_;

  //
  const bool fast_mode_;

  //
  const ModificationType modification_type_;

  //
  FastFourierTransformForRealSequence* fourier_transform_;

  //
  InverseFastFourierTransform* inverse_fourier_transform_;

  //
  bool is_valid_;

  //
  DISALLOW_COPY_AND_ASSIGN(MlsaDigitalFilterStabilityCheck);
};

}  // namespace sptk

#endif  // SPTK_UTILS_MLSA_DIGITAL_FILTER_STABILITY_CHECK_H_
