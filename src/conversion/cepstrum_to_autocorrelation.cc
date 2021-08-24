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

#include "SPTK/conversion/cepstrum_to_autocorrelation.h"

#include <algorithm>  // std::copy, std::fill, std::transform
#include <cmath>      // std::exp
#include <cstddef>    // std::size_t

namespace sptk {

CepstrumToAutocorrelation::CepstrumToAutocorrelation(int num_input_order,
                                                     int num_output_order,
                                                     int fft_length)
    : num_input_order_(num_input_order),
      num_output_order_(num_output_order),
      fast_fourier_transform_(fft_length),
      is_valid_(true) {
  if (num_input_order_ < 0 || num_output_order_ < 0 ||
      fft_length <= num_input_order_ || fft_length <= num_output_order_ ||
      !fast_fourier_transform_.IsValid()) {
    is_valid_ = false;
    return;
  }
}

bool CepstrumToAutocorrelation::Run(
    const std::vector<double>& cepstrum, std::vector<double>* autocorrelation,
    CepstrumToAutocorrelation::Buffer* buffer) const {
  // Check inputs.
  const int input_length(num_input_order_ + 1);
  if (!is_valid_ || cepstrum.size() != static_cast<std::size_t>(input_length) ||
      NULL == autocorrelation || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  const int output_length(num_output_order_ + 1);
  if (autocorrelation->size() != static_cast<std::size_t>(output_length)) {
    autocorrelation->resize(output_length);
  }
  const int fft_length(fast_fourier_transform_.GetFftLength());
  if (buffer->real_part_.size() != static_cast<std::size_t>(fft_length)) {
    buffer->real_part_.resize(fft_length);
  }

  std::copy(cepstrum.begin(), cepstrum.end(), buffer->real_part_.begin());
  std::fill(buffer->real_part_.begin() + input_length, buffer->real_part_.end(),
            0.0);

  if (!fast_fourier_transform_.Run(
          &buffer->real_part_, &buffer->imag_part_,
          &buffer->buffer_for_fast_fourier_transform_)) {
    return false;
  }

  std::transform(buffer->real_part_.begin(), buffer->real_part_.end(),
                 buffer->real_part_.begin(),
                 [](double x) { return std::exp(2.0 * x); });

  if (!fast_fourier_transform_.Run(
          &buffer->real_part_, &buffer->imag_part_,
          &buffer->buffer_for_fast_fourier_transform_)) {
    return false;
  }

  const double z(1.0 / fft_length);
  std::transform(buffer->real_part_.begin(),
                 buffer->real_part_.begin() + output_length,
                 autocorrelation->begin(), [z](double x) { return x * z; });

  return true;
}

}  // namespace sptk
