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
#include <vector>     // std::vector

namespace sptk {

MelCepstrumPowerNormalization::MelCepstrumPowerNormalization(
    int num_order, int impulse_response_length, double alpha)
    : frequency_transform_(num_order, impulse_response_length - 1, -alpha),
      cepstrum_to_autocorrelation_(impulse_response_length - 1, 0,
                                   impulse_response_length),
      is_valid_(true) {
  if (!frequency_transform_.IsValid() ||
      !cepstrum_to_autocorrelation_.IsValid()) {
    is_valid_ = false;
    return;
  }
}

bool MelCepstrumPowerNormalization::Run(
    const std::vector<double>& mel_cepstrum,
    std::vector<double>* power_normalized_mel_cepstrum, double* power,
    MelCepstrumPowerNormalization::Buffer* buffer) const {
  // Check inputs.
  const int length(GetNumOrder() + 1);
  if (!is_valid_ || mel_cepstrum.size() != static_cast<std::size_t>(length) ||
      NULL == power_normalized_mel_cepstrum || NULL == power ||
      NULL == buffer) {
    return false;
  }

  // Prepare memories.
  if (power_normalized_mel_cepstrum->size() !=
      static_cast<std::size_t>(length)) {
    power_normalized_mel_cepstrum->resize(length);
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
  *power = std::log(buffer->autocorrelation_[0]);
  (*power_normalized_mel_cepstrum)[0] = mel_cepstrum[0] - 0.5 * (*power);
  std::copy(mel_cepstrum.begin() + 1, mel_cepstrum.end(),
            power_normalized_mel_cepstrum->begin() + 1);

  return true;
}

bool MelCepstrumPowerNormalization::Run(
    std::vector<double>* input_and_output, double* power,
    MelCepstrumPowerNormalization::Buffer* buffer) const {
  if (NULL == input_and_output) return false;
  return Run(*input_and_output, input_and_output, power, buffer);
}

}  // namespace sptk
