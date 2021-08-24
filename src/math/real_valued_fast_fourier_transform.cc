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

#include "SPTK/math/real_valued_fast_fourier_transform.h"

#include <algorithm>  // std::fill
#include <cmath>      // std::sin
#include <cstddef>    // std::size_t

namespace sptk {

RealValuedFastFourierTransform::RealValuedFastFourierTransform(int fft_length)
    : RealValuedFastFourierTransform(fft_length - 1, fft_length) {
}

RealValuedFastFourierTransform::RealValuedFastFourierTransform(int num_order,
                                                               int fft_length)
    : num_order_(num_order),
      fft_length_(fft_length),
      half_fft_length_(fft_length_ / 2),
      fast_fourier_transform_(half_fft_length_),
      is_valid_(true) {
  if (num_order_ < 0 || fft_length_ <= num_order_ ||
      !IsPowerOfTwo(fft_length_) || !fast_fourier_transform_.IsValid()) {
    is_valid_ = false;
    return;
  }

  const int table_size(fft_length_ - fft_length_ / 4 + 1);
  const double argument(sptk::kPi / fft_length_ * 2);
  sine_table_.resize(table_size);
  for (int i(0); i < table_size; ++i) {
    sine_table_[i] = std::sin(argument * i);
  }
  sine_table_[fft_length_ / 2] = 0.0;
}

bool RealValuedFastFourierTransform::Run(
    const std::vector<double>& real_part_input,
    std::vector<double>* real_part_output,
    std::vector<double>* imag_part_output,
    RealValuedFastFourierTransform::Buffer* buffer) const {
  // Check inputs.
  const int input_length(num_order_ + 1);
  if (!is_valid_ ||
      real_part_input.size() != static_cast<std::size_t>(input_length) ||
      NULL == real_part_output || NULL == imag_part_output || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  if (buffer->real_part_input_.size() !=
      static_cast<std::size_t>(half_fft_length_)) {
    buffer->real_part_input_.resize(half_fft_length_);
  }
  if (buffer->imag_part_input_.size() !=
      static_cast<std::size_t>(half_fft_length_)) {
    buffer->imag_part_input_.resize(half_fft_length_);
  }
  if (real_part_output->capacity() < static_cast<std::size_t>(fft_length_)) {
    real_part_output->reserve(fft_length_);
  }
  if (imag_part_output->capacity() < static_cast<std::size_t>(fft_length_)) {
    imag_part_output->reserve(fft_length_);
  }

  // Copy input and fill zero.
  for (int i(0), j(0); i < input_length; ++j) {
    buffer->real_part_input_[j] = real_part_input[i++];
    if (input_length <= i) break;
    buffer->imag_part_input_[j] = real_part_input[i++];
  }
  std::fill(buffer->real_part_input_.begin() + (input_length + 1) / 2,
            buffer->real_part_input_.end(), 0.0);
  std::fill(buffer->imag_part_input_.begin() + input_length / 2,
            buffer->imag_part_input_.end(), 0.0);

  // Run fast Fourier transform.
  if (!fast_fourier_transform_.Run(buffer->real_part_input_,
                                   buffer->imag_part_input_, real_part_output,
                                   imag_part_output)) {
    return false;
  }
  real_part_output->resize(fft_length_);
  imag_part_output->resize(fft_length_);

  double* x(&((*real_part_output)[0]));
  double* y(&((*imag_part_output)[0]));
  double* xp(x);
  double* yp(y);
  double* xq(xp + fft_length_);
  double* yq(yp + fft_length_);
  *(xp + half_fft_length_) = *xp - *yp;
  *xp = *xp + *yp;
  *(yp + half_fft_length_) = 0.0;
  *yp = 0.0;

  const double* sinp(&(sine_table_[0]));
  const double* cosp(&(sine_table_[0]) + fft_length_ / 4);
  for (int i(1), j(half_fft_length_ - 2); i < half_fft_length_; ++i, j -= 2) {
    ++xp;
    ++yp;
    ++sinp;
    ++cosp;
    const double xt(*xp - *(xp + j));
    const double yt(*yp + *(yp + j));
    *(--xq) = (*xp + *(xp + j) + *cosp * yt - *sinp * xt) * 0.5;
    *(--yq) = (-*yp + *(yp + j) + *sinp * yt + *cosp * xt) * 0.5;
  }

  xp = x + 1;
  yp = y + 1;
  xq = x + fft_length_;
  yq = y + fft_length_;
  for (int i(1); i < half_fft_length_; ++i) {
    *xp++ = *(--xq);
    *yp++ = -(*(--yq));
  }

  return true;
}

bool RealValuedFastFourierTransform::Run(
    std::vector<double>* real_part, std::vector<double>* imag_part,
    RealValuedFastFourierTransform::Buffer* buffer) const {
  if (NULL == real_part) return false;
  return Run(*real_part, real_part, imag_part, buffer);
}

}  // namespace sptk
