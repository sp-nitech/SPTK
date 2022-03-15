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

#include "SPTK/conversion/spectrum_to_autocorrelation.h"

#include <algorithm>  // std::copy, std::reverse_copy
#include <cstddef>    // std::size_t

namespace sptk {

SpectrumToAutocorrelation::SpectrumToAutocorrelation(int fft_length,
                                                     int num_order)
    : num_order_(num_order),
      inverse_fast_fourier_transform_(fft_length),
      is_valid_(true) {
  if (num_order_ < 0 || fft_length < 2 * num_order_ ||
      !inverse_fast_fourier_transform_.IsValid()) {
    is_valid_ = false;
    return;
  }
}

bool SpectrumToAutocorrelation::Run(
    const std::vector<double>& power_spectrum,
    std::vector<double>* autocorrelation,
    SpectrumToAutocorrelation::Buffer* buffer) const {
  // Check inputs.
  const int fft_length(GetFftLength());
  if (!is_valid_ ||
      power_spectrum.size() != static_cast<std::size_t>(fft_length / 2 + 1) ||
      NULL == autocorrelation || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  if (autocorrelation->size() != static_cast<std::size_t>(num_order_ + 1)) {
    autocorrelation->resize(num_order_ + 1);
  }
  if (buffer->real_part_.size() != static_cast<std::size_t>(fft_length)) {
    buffer->real_part_.resize(fft_length);
  }

  // Make full power spectrum.
  std::copy(power_spectrum.begin(), power_spectrum.end(),
            buffer->real_part_.begin());
  std::reverse_copy(buffer->real_part_.begin() + 1,
                    buffer->real_part_.begin() + power_spectrum.size() - 1,
                    buffer->real_part_.begin() + power_spectrum.size());

  // Obtain autocorrelation.
  if (!inverse_fast_fourier_transform_.Run(
          &buffer->real_part_, &buffer->imag_part_,
          &buffer->buffer_for_inverse_fast_fourier_transform_)) {
    return false;
  }

  // Save outputs.
  std::copy(buffer->real_part_.begin(),
            buffer->real_part_.begin() + autocorrelation->size(),
            autocorrelation->begin());

  return true;
}

}  // namespace sptk
