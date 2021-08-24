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

#include "SPTK/conversion/cepstrum_to_negative_derivative_of_phase_spectrum.h"

#include <algorithm>  // std::fill, std::reverse_copy
#include <cstddef>    // std::size_t

namespace sptk {

CepstrumToNegativeDerivativeOfPhaseSpectrum::
    CepstrumToNegativeDerivativeOfPhaseSpectrum(int num_order, int fft_length)
    : num_order_(num_order),
      fast_fourier_transform_(fft_length),
      is_valid_(true) {
  if (num_order_ < 0 || fft_length < 2 * num_order_ ||
      !fast_fourier_transform_.IsValid()) {
    is_valid_ = false;
    return;
  }
}

bool CepstrumToNegativeDerivativeOfPhaseSpectrum::Run(
    const std::vector<double>& cepstrum,
    std::vector<double>* negative_derivative_of_phase_spectrum,
    CepstrumToNegativeDerivativeOfPhaseSpectrum::Buffer* buffer) const {
  // Check inputs.
  if (!is_valid_ ||
      cepstrum.size() != static_cast<std::size_t>(num_order_ + 1) ||
      NULL == negative_derivative_of_phase_spectrum || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  const int fft_length(fast_fourier_transform_.GetFftLength());
  if (buffer->fast_fourier_transform_real_part_.size() !=
      static_cast<std::size_t>(fft_length)) {
    buffer->fast_fourier_transform_real_part_.resize(fft_length);
  }

  const double* c(&(cepstrum[0]));
  double* v(&buffer->fast_fourier_transform_real_part_[0]);

  // Fill the left side of the input with cepstrum.
  v[0] = 0.0;
  for (int m(1); m <= num_order_; ++m) {
    v[m] = 0.5 * m * c[m];
  }
  const int half_fft_length(fft_length / 2);
  std::fill(
      buffer->fast_fourier_transform_real_part_.begin() + num_order_ + 1,
      buffer->fast_fourier_transform_real_part_.begin() + half_fft_length + 1,
      0.0);

  // Fill the right side of the input by mirroring the left side.
  std::reverse_copy(
      buffer->fast_fourier_transform_real_part_.begin() + 1,
      buffer->fast_fourier_transform_real_part_.begin() + half_fft_length + 1,
      buffer->fast_fourier_transform_real_part_.begin() + half_fft_length);

  if (half_fft_length == num_order_) {
    // Double the value at the center because the cepstrum values on both
    // sides are overlapped at the center.
    v[num_order_] *= 2.0;
  }

  if (!fast_fourier_transform_.Run(buffer->fast_fourier_transform_real_part_,
                                   negative_derivative_of_phase_spectrum,
                                   &buffer->fast_fourier_transform_imag_part_,
                                   &buffer->fast_fourier_transform_buffer_)) {
    return false;
  }

  return true;
}

}  // namespace sptk
