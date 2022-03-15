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

#include "SPTK/analysis/fast_fourier_transform_cepstral_analysis.h"

#include <algorithm>  // std::reverse_copy, std::transform
#include <cmath>      // std::log
#include <cstddef>    // std::size_t

namespace sptk {

FastFourierTransformCepstralAnalysis::FastFourierTransformCepstralAnalysis(
    int fft_length, int num_order, int num_iteration,
    double acceleration_factor)
    : num_order_(num_order),
      num_iteration_(num_iteration),
      acceleration_factor_(acceleration_factor),
      fast_fourier_transform_(fft_length),
      inverse_fast_fourier_transform_(fft_length),
      is_valid_(true) {
  if (num_order_ < 0 || fft_length < 2 * num_order_ || num_iteration_ < 0 ||
      acceleration_factor_ < 0.0 || !fast_fourier_transform_.IsValid() ||
      !inverse_fast_fourier_transform_.IsValid()) {
    is_valid_ = false;
    return;
  }
}

bool FastFourierTransformCepstralAnalysis::Run(
    const std::vector<double>& power_spectrum, std::vector<double>* cepstrum,
    FastFourierTransformCepstralAnalysis::Buffer* buffer) const {
  // Check inputs.
  const int fft_length(GetFftLength());
  if (!is_valid_ ||
      power_spectrum.size() != static_cast<std::size_t>(fft_length / 2 + 1) ||
      NULL == cepstrum || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  if (cepstrum->size() != static_cast<std::size_t>(num_order_ + 1)) {
    cepstrum->resize(num_order_ + 1);
  }
  if (buffer->real_part_.size() != static_cast<std::size_t>(fft_length)) {
    buffer->real_part_.resize(fft_length);
  }

  // Make full log-power spectrum.
  std::transform(power_spectrum.begin(), power_spectrum.end(),
                 buffer->real_part_.begin(),
                 [](double x) { return std::log(x); });
  std::reverse_copy(buffer->real_part_.begin() + 1,
                    buffer->real_part_.begin() + power_spectrum.size() - 1,
                    buffer->real_part_.begin() + power_spectrum.size());

  // Obtain smoothed cepstrum.
  if (!inverse_fast_fourier_transform_.Run(
          &buffer->real_part_, &buffer->imag_part_,
          &buffer->buffer_for_inverse_fast_fourier_transform_)) {
    return false;
  }

  // Initialize variables.
  double* v(&((*cepstrum)[0]));
  double* e(&buffer->real_part_[0]);
  for (int m(0); m <= num_order_; ++m) {
    v[m] = e[m];
    e[m] = 0.0;
  }

  // Perform the improved cepstral analysis method.
  for (int i(0); i < num_iteration_; ++i) {
    for (int m(1); m <= num_order_; ++m) {
      e[fft_length - m] = e[m];
    }

    // Ek = DFT(em).
    if (!fast_fourier_transform_.Run(
            &buffer->real_part_, &buffer->imag_part_,
            &buffer->buffer_for_fast_fourier_transform_)) {
      return false;
    }

    // Apply function g.
    for (int k(0); k < fft_length; ++k) {
      if (e[k] < 0.0) {
        e[k] = 0.0;
      }
    }

    // em = IDFT(Ek).
    if (!inverse_fast_fourier_transform_.Run(
            &buffer->real_part_, &buffer->imag_part_,
            &buffer->buffer_for_inverse_fast_fourier_transform_)) {
      return false;
    }

    // Update vm and em.
    for (int m(0); m <= num_order_; ++m) {
      const double tm(e[m] * (1.0 + acceleration_factor_));
      v[m] += tm;
      e[m] -= tm;
    }
  }

  v[0] *= 0.5;
  if (fft_length / 2 == num_order_) {
    v[num_order_] *= 0.5;
  }

  return true;
}

}  // namespace sptk
