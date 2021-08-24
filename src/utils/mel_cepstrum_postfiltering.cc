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

#include "SPTK/utils/mel_cepstrum_postfiltering.h"

#include <algorithm>  // std::copy, std::transform
#include <cmath>      // std::log
#include <cstddef>    // std::size_t

namespace sptk {

MelCepstrumPostfiltering::MelCepstrumPostfiltering(int num_order,
                                                   int impulse_response_length,
                                                   int onset_index,
                                                   double alpha, double beta)
    : onset_index_(onset_index),
      beta_(beta),
      frequency_transform_(num_order, impulse_response_length - 1, -alpha),
      cepstrum_to_autocorrelation_(impulse_response_length - 1, 0,
                                   impulse_response_length),
      mel_cepstrum_to_mlsa_digital_filter_coefficients_(num_order, alpha),
      mlsa_digital_filter_coefficients_to_mel_cepstrum_(num_order, alpha),
      is_valid_(true) {
  if (!IsInRange(num_order, 0, impulse_response_length - 1) ||
      !IsInRange(onset_index_, 0, num_order) ||
      !frequency_transform_.IsValid() ||
      !cepstrum_to_autocorrelation_.IsValid() ||
      !mel_cepstrum_to_mlsa_digital_filter_coefficients_.IsValid() ||
      !mlsa_digital_filter_coefficients_to_mel_cepstrum_.IsValid()) {
    is_valid_ = false;
    return;
  }
}

bool MelCepstrumPostfiltering::Run(
    const std::vector<double>& mel_cepstrum,
    std::vector<double>* postfiltered_mel_cepstrum,
    MelCepstrumPostfiltering::Buffer* buffer) const {
  // Check inputs.
  const int length(GetNumOrder() + 1);
  if (!is_valid_ || mel_cepstrum.size() != static_cast<std::size_t>(length) ||
      NULL == postfiltered_mel_cepstrum || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  if (postfiltered_mel_cepstrum->size() != static_cast<std::size_t>(length)) {
    postfiltered_mel_cepstrum->resize(length);
  }
  if (buffer->mel_cepstrum_.size() != static_cast<std::size_t>(length)) {
    buffer->mel_cepstrum_.resize(length);
  }

  // Handle special case.
  if (0.0 == beta_) {
    std::copy(mel_cepstrum.begin(), mel_cepstrum.end(),
              postfiltered_mel_cepstrum->begin());
    return true;
  }

  // Calculate energy of original mel-cepstrum.
  {
    if (!frequency_transform_.Run(mel_cepstrum, &buffer->cepstrum_,
                                  &buffer->buffer_for_frequency_transform_)) {
      return false;
    }
    if (!cepstrum_to_autocorrelation_.Run(
            buffer->cepstrum_, &buffer->autocorrelation_,
            &buffer->buffer_for_cepstrum_to_autocorrelation_)) {
      return false;
    }
  }
  const double original_energy(buffer->autocorrelation_[0]);

  // Calculate energy of modified mel-cepstrum.
  {
    std::copy(mel_cepstrum.begin(), mel_cepstrum.begin() + onset_index_,
              buffer->mel_cepstrum_.begin());
    const double weight(1.0 + beta_);
    std::transform(mel_cepstrum.begin() + onset_index_, mel_cepstrum.end(),
                   buffer->mel_cepstrum_.begin() + onset_index_,
                   [weight](double c) { return c * weight; });

    if (!frequency_transform_.Run(buffer->mel_cepstrum_, &buffer->cepstrum_,
                                  &buffer->buffer_for_frequency_transform_)) {
      return false;
    }
    if (!cepstrum_to_autocorrelation_.Run(
            buffer->cepstrum_, &buffer->autocorrelation_,
            &buffer->buffer_for_cepstrum_to_autocorrelation_)) {
      return false;
    }
  }
  const double modified_energy(buffer->autocorrelation_[0]);

  // Adjust gain.
  {
    if (!mel_cepstrum_to_mlsa_digital_filter_coefficients_.Run(
            buffer->mel_cepstrum_,
            &buffer->mlsa_digital_filter_coefficients_)) {
      return false;
    }

    buffer->mlsa_digital_filter_coefficients_[0] +=
        (0.5 * std::log(original_energy / modified_energy));

    if (!mlsa_digital_filter_coefficients_to_mel_cepstrum_.Run(
            buffer->mlsa_digital_filter_coefficients_,
            postfiltered_mel_cepstrum)) {
      return false;
    }
  }

  return true;
}

bool MelCepstrumPostfiltering::Run(
    std::vector<double>* input_and_output,
    MelCepstrumPostfiltering::Buffer* buffer) const {
  if (NULL == input_and_output) return false;
  return Run(*input_and_output, input_and_output, buffer);
}

}  // namespace sptk
