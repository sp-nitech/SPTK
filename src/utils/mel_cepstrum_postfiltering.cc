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
