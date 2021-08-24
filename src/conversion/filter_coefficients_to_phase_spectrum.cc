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

#include "SPTK/conversion/filter_coefficients_to_phase_spectrum.h"

#include <algorithm>  // std::copy, std::fill
#include <cmath>      // std::atan2
#include <cstddef>    // std::size_t

namespace sptk {

FilterCoefficientsToPhaseSpectrum::FilterCoefficientsToPhaseSpectrum(
    int num_numerator_order, int num_denominator_order, int fft_length,
    bool unwrapping)
    : num_numerator_order_(num_numerator_order),
      num_denominator_order_(num_denominator_order),
      fft_length_(fft_length),
      unwrapping_(unwrapping),
      fast_fourier_transform_(fft_length),
      is_valid_(true) {
  if (num_numerator_order_ < 0 || num_denominator_order_ < 0 ||
      fft_length_ <= num_numerator_order_ ||
      fft_length_ <= num_denominator_order_ ||
      !fast_fourier_transform_.IsValid()) {
    is_valid_ = false;
    return;
  }
}

bool FilterCoefficientsToPhaseSpectrum::Run(
    const std::vector<double>& numerator_coefficients,
    const std::vector<double>& denominator_coefficients,
    std::vector<double>* phase_spectrum,
    FilterCoefficientsToPhaseSpectrum::Buffer* buffer) const {
  // Check inputs.
  const int numerator_length(num_numerator_order_ + 1);
  const int denominator_length(num_denominator_order_ + 1);
  if (!is_valid_ ||
      numerator_coefficients.size() !=
          static_cast<std::size_t>(numerator_length) ||
      denominator_coefficients.size() !=
          static_cast<std::size_t>(denominator_length) ||
      NULL == phase_spectrum || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  const int output_length(fft_length_ / 2 + 1);
  if (phase_spectrum->size() != static_cast<std::size_t>(output_length)) {
    phase_spectrum->resize(output_length);
  }
  if (buffer->real_part1_.size() != static_cast<std::size_t>(fft_length_)) {
    buffer->real_part1_.resize(fft_length_);
  }
  if (buffer->real_part2_.size() != static_cast<std::size_t>(fft_length_)) {
    buffer->real_part2_.resize(fft_length_);
  }

  std::copy(numerator_coefficients.begin(), numerator_coefficients.end(),
            buffer->real_part1_.begin());
  std::fill(buffer->real_part1_.begin() + numerator_length,
            buffer->real_part1_.end(), 0.0);
  std::copy(denominator_coefficients.begin(), denominator_coefficients.end(),
            buffer->real_part2_.begin());
  std::fill(buffer->real_part2_.begin() + denominator_length,
            buffer->real_part2_.end(), 0.0);

  if (!fast_fourier_transform_.Run(
          &buffer->real_part1_, &buffer->imag_part1_,
          &buffer->buffer_for_fast_fourier_transform_)) {
    return false;
  }

  buffer->real_part2_[0] = 1.0;
  if (!fast_fourier_transform_.Run(
          &buffer->real_part2_, &buffer->imag_part2_,
          &buffer->buffer_for_fast_fourier_transform_)) {
    return false;
  }

  double* output(&((*phase_spectrum)[0]));
  double* xr(&buffer->real_part1_[0]);
  double* yr(&buffer->real_part2_[0]);
  double* xi(&buffer->imag_part1_[0]);
  double* yi(&buffer->imag_part2_[0]);

  const double inverse_pi(1.0 / sptk::kPi);
  double offset(0.0);

  for (int i(0); i < output_length; ++i) {
    output[i] = inverse_pi * std::atan2(xi[i] * yr[i] - xr[i] * yi[i],
                                        xr[i] * yr[i] + xi[i] * yi[i]);
    if (unwrapping_ && 0 < i) {
      const double diff(output[i] - output[i - 1] + offset);
      if (1.0 < diff) {
        offset -= 2.0;
      } else if (1.0 < -diff) {
        offset += 2.0;
      }
      output[i] += offset;
    }
  }

  return true;
}

}  // namespace sptk
