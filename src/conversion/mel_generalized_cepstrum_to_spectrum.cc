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
