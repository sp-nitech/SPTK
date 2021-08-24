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

#include "SPTK/conversion/negative_derivative_of_phase_spectrum_to_cepstrum.h"

#include <algorithm>  // std::copy, std::reverse_copy
#include <cstddef>    // std::size_t

namespace sptk {

NegativeDerivativeOfPhaseSpectrumToCepstrum::
    NegativeDerivativeOfPhaseSpectrumToCepstrum(int fft_length, int num_order)
    : num_order_(num_order),
      fast_fourier_transform_(fft_length),
      is_valid_(true) {
  if (num_order_ < 0 || fft_length < 2 * num_order_ ||
      !fast_fourier_transform_.IsValid()) {
    is_valid_ = false;
    return;
  }
}

bool NegativeDerivativeOfPhaseSpectrumToCepstrum::Run(
    const std::vector<double>& negative_derivative_of_phase_spectrum,
    std::vector<double>* cepstrum,
    NegativeDerivativeOfPhaseSpectrumToCepstrum::Buffer* buffer) const {
  // Check inputs.
  const int fft_length(fast_fourier_transform_.GetFftLength());
  const int half_fft_length(fft_length / 2);
  if (!is_valid_ ||
      negative_derivative_of_phase_spectrum.size() !=
          static_cast<std::size_t>(half_fft_length + 1) ||
      NULL == cepstrum || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  if (cepstrum->size() != static_cast<std::size_t>(num_order_ + 1)) {
    cepstrum->resize(num_order_ + 1);
  }
  if (buffer->fast_fourier_transform_real_part_.size() !=
      static_cast<std::size_t>(fft_length)) {
    buffer->fast_fourier_transform_real_part_.resize(fft_length);
  }

  std::copy(negative_derivative_of_phase_spectrum.begin(),
            negative_derivative_of_phase_spectrum.end(),
            buffer->fast_fourier_transform_real_part_.begin());
  std::reverse_copy(
      negative_derivative_of_phase_spectrum.begin() + 1,
      negative_derivative_of_phase_spectrum.end(),
      buffer->fast_fourier_transform_real_part_.begin() + half_fft_length);

  if (!fast_fourier_transform_.Run(&buffer->fast_fourier_transform_real_part_,
                                   &buffer->fast_fourier_transform_imag_part_,
                                   &buffer->fast_fourier_transform_buffer_)) {
    return false;
  }

  double* v(&buffer->fast_fourier_transform_real_part_[0]);
  double* c(&((*cepstrum)[0]));

  c[0] = 0.0;
  for (int m(1); m <= num_order_; ++m) {
    c[m] = v[m] / (m * half_fft_length);
  }
  if (half_fft_length == num_order_) {
    c[num_order_] *= 0.5;
  }

  return true;
}

}  // namespace sptk
