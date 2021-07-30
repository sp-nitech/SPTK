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
//                1996-2021  Nagoya Institute of Technology          //
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

#include "SPTK/conversion/mel_generalized_cepstrum_to_spectrum.h"

namespace sptk {

MelGeneralizedCepstrumToSpectrum::MelGeneralizedCepstrumToSpectrum(
    int num_order, double alpha, double gamma, bool is_normalized,
    bool is_multiplied, int fft_length)
    : mel_generalized_cepstrum_transform_(
          num_order, alpha, gamma, is_normalized, is_multiplied, fft_length / 2,
          0.0, 0.0, false, false),
      fast_fourier_transform_(fft_length / 2, fft_length),
      is_valid_(true) {
  if (!mel_generalized_cepstrum_transform_.IsValid() ||
      !fast_fourier_transform_.IsValid()) {
    is_valid_ = false;
    return;
  }
}

bool MelGeneralizedCepstrumToSpectrum::Run(
    const std::vector<double>& mel_generalized_cepstrum,
    std::vector<double>* amplitude_spectrum,
    std::vector<double>* phase_spectrum,
    MelGeneralizedCepstrumToSpectrum::Buffer* buffer) const {
  if (!is_valid_ || NULL == amplitude_spectrum || NULL == phase_spectrum ||
      NULL == buffer) {
    return false;
  }

  if (!mel_generalized_cepstrum_transform_.Run(
          mel_generalized_cepstrum, &buffer->cepstrum_,
          &buffer->mel_generalized_cepstrum_transform_buffer_)) {
    return false;
  }

  if (!fast_fourier_transform_.Run(buffer->cepstrum_, amplitude_spectrum,
                                   phase_spectrum,
                                   &buffer->fast_fourier_transform_buffer_)) {
    return false;
  }

  return true;
}

}  // namespace sptk
