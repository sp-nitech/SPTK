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

#include "SPTK/conversion/filter_coefficients_to_group_delay.h"

#include <algorithm>  // std::copy, std::fill, std::reverse_copy
#include <cmath>      // std::pow
#include <cstddef>    // std::size_t

#include "SPTK/utils/misc_utils.h"

namespace sptk {

FilterCoefficientsToGroupDelay::FilterCoefficientsToGroupDelay(
    int num_numerator_order, int num_denominator_order, int fft_length,
    double alpha, double gamma)
    : num_numerator_order_(num_numerator_order),
      num_denominator_order_(num_denominator_order),
      fft_length_(fft_length),
      alpha_(alpha),
      gamma_(gamma),
      fast_fourier_transform_(fft_length),
      is_valid_(true) {
  if (num_numerator_order_ < 0 || num_denominator_order_ < 0 ||
      fft_length_ < num_numerator_order_ + num_denominator_order_ + 1 ||
      alpha_ <= 0.0 || gamma_ <= 0.0 || !fast_fourier_transform_.IsValid()) {
    is_valid_ = false;
    return;
  }
}

bool FilterCoefficientsToGroupDelay::Run(
    const std::vector<double>& numerator_coefficients,
    const std::vector<double>& denominator_coefficients,
    std::vector<double>* group_delay,
    FilterCoefficientsToGroupDelay::Buffer* buffer) const {
  // Check inputs.
  const int numerator_length(num_numerator_order_ + 1);
  const int denominator_length(num_denominator_order_ + 1);
  if (!is_valid_ ||
      numerator_coefficients.size() !=
          static_cast<std::size_t>(numerator_length) ||
      denominator_coefficients.size() !=
          static_cast<std::size_t>(denominator_length) ||
      NULL == group_delay || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  const int output_length(fft_length_ / 2 + 1);
  if (group_delay->size() != static_cast<std::size_t>(output_length)) {
    group_delay->resize(output_length);
  }
  if (buffer->real_part1_.size() != static_cast<std::size_t>(fft_length_)) {
    buffer->real_part1_.resize(fft_length_);
  }
  if (buffer->real_part2_.size() != static_cast<std::size_t>(fft_length_)) {
    buffer->real_part2_.resize(fft_length_);
  }
  if (buffer->flipped_coefficients_.size() !=
      static_cast<std::size_t>(num_denominator_order_ + 1)) {
    buffer->flipped_coefficients_.resize(num_denominator_order_ + 1);
  }

  std::reverse_copy(denominator_coefficients.begin() + 1,
                    denominator_coefficients.end(),
                    buffer->flipped_coefficients_.begin());
  buffer->flipped_coefficients_.back() = 1.0;

  if (!Perform1DConvolution(numerator_coefficients,
                            buffer->flipped_coefficients_,
                            &buffer->convolved_coefficients_)) {
    return false;
  }
  const int coefficients_length(
      static_cast<int>(buffer->convolved_coefficients_.size()));

  std::copy(buffer->convolved_coefficients_.begin(),
            buffer->convolved_coefficients_.end(), buffer->real_part1_.begin());
  std::fill(buffer->real_part1_.begin() + coefficients_length,
            buffer->real_part1_.end(), 0.0);

  // Make ramped polynomial.
  {
    double* src(&buffer->convolved_coefficients_[0]);
    double* dst(&buffer->real_part2_[0]);
    for (int i(0); i < coefficients_length; ++i) {
      dst[i] = i * src[i];
    }
  }
  std::fill(buffer->real_part2_.begin() + coefficients_length,
            buffer->real_part2_.end(), 0.0);

  if (!fast_fourier_transform_.Run(
          &buffer->real_part1_, &buffer->imag_part1_,
          &buffer->buffer_for_fast_fourier_transform_)) {
    return false;
  }

  if (!fast_fourier_transform_.Run(
          &buffer->real_part2_, &buffer->imag_part2_,
          &buffer->buffer_for_fast_fourier_transform_)) {
    return false;
  }

  double* output(&((*group_delay)[0]));
  double* xr(&buffer->real_part1_[0]);
  double* yr(&buffer->real_part2_[0]);
  double* xi(&buffer->imag_part1_[0]);
  double* yi(&buffer->imag_part2_[0]);

  for (int i(0); i < output_length; ++i) {
    double denominator(xr[i] * xr[i] + xi[i] * xi[i]);
    if (1.0 != gamma_) {
      denominator = std::pow(denominator, gamma_);
    }
    double numerator(xr[i] * yr[i] + xi[i] * yi[i]);
    output[i] = numerator / denominator - num_denominator_order_;
    if (1.0 != alpha_) {
      output[i] = (0.0 < output[i]) ? std::pow(output[i], alpha_)
                                    : -std::pow(-output[i], alpha_);
    }
  }

  return true;
}

}  // namespace sptk
