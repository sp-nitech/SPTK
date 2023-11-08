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

#include "SPTK/conversion/mel_cepstrum_power_normalization.h"

#include <algorithm>  // std::copy
#include <cmath>      // std::log
#include <cstddef>    // std::size_t

namespace sptk {

MelCepstrumPowerNormalization::MelCepstrumPowerNormalization(
    int num_order, int impulse_response_length, double alpha)
    : frequency_transform_(num_order, impulse_response_length - 1, -alpha),
      cepstrum_to_autocorrelation_(impulse_response_length - 1, 0,
                                   impulse_response_length),
      is_valid_(true) {
  if (!sptk::IsInRange(num_order, 0, impulse_response_length - 1) ||
      !frequency_transform_.IsValid() ||
      !cepstrum_to_autocorrelation_.IsValid()) {
    is_valid_ = false;
    return;
  }
}

bool MelCepstrumPowerNormalization::Run(
    const std::vector<double>& mel_cepstrum,
    std::vector<double>* power_normalized_mel_cepstrum,
    MelCepstrumPowerNormalization::Buffer* buffer) const {
  // Check inputs.
  const int length(GetNumOrder() + 1);
  if (!is_valid_ || mel_cepstrum.size() != static_cast<std::size_t>(length) ||
      NULL == power_normalized_mel_cepstrum || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  if (power_normalized_mel_cepstrum->size() !=
      static_cast<std::size_t>(length + 1)) {
    power_normalized_mel_cepstrum->resize(length + 1);
  }

  // Calculate power of input mel-cepstrum.
  if (!frequency_transform_.Run(mel_cepstrum, &buffer->cepstrum_,
                                &buffer->buffer_for_frequency_transform_)) {
    return false;
  }
  if (!cepstrum_to_autocorrelation_.Run(
          buffer->cepstrum_, &buffer->autocorrelation_,
          &buffer->buffer_for_cepstrum_to_autocorrelation_)) {
    return false;
  }

  // Convert.
  const double log_k(0.5 * std::log(buffer->autocorrelation_[0]));
  (*power_normalized_mel_cepstrum)[0] = log_k;
  (*power_normalized_mel_cepstrum)[1] = mel_cepstrum[0] - log_k;
  std::copy(mel_cepstrum.begin() + 1, mel_cepstrum.end(),
            power_normalized_mel_cepstrum->begin() + 2);

  return true;
}

}  // namespace sptk
