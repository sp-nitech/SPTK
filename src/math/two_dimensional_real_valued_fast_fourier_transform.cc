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

#include "SPTK/math/two_dimensional_real_valued_fast_fourier_transform.h"

#include <algorithm>  // std::copy, std::fill
#include <cstddef>    // std::size_t

namespace sptk {

TwoDimensionalRealValuedFastFourierTransform::
    TwoDimensionalRealValuedFastFourierTransform(int num_row, int num_column,
                                                 int fft_length)
    : num_row_(num_row),
      num_column_(num_column),
      fft_length_(fft_length),
      fast_fourier_transform_(fft_length_),
      real_valued_fast_fourier_transform_(fft_length_),
      is_valid_(true) {
  if (num_row_ <= 0 || fft_length_ < num_row_ || num_column_ <= 0 ||
      fft_length < num_column_ || !fast_fourier_transform_.IsValid() ||
      !real_valued_fast_fourier_transform_.IsValid()) {
    is_valid_ = false;
    return;
  }
}

bool TwoDimensionalRealValuedFastFourierTransform::Run(
    const Matrix& real_part_input, Matrix* real_part_output,
    Matrix* imag_part_output,
    TwoDimensionalRealValuedFastFourierTransform::Buffer* buffer) const {
  // Check inputs.
  if (!is_valid_ || real_part_input.GetNumRow() != num_row_ ||
      real_part_input.GetNumColumn() != num_column_ ||
      NULL == real_part_output || NULL == imag_part_output || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  {
    const std::size_t size(static_cast<std::size_t>(fft_length_));
    if (buffer->real_part_input_.size() != size) {
      buffer->real_part_input_.resize(size);
    }
    if (buffer->imag_part_input_.size() != size) {
      buffer->imag_part_input_.resize(size);
    }
    if (buffer->first_real_part_outputs_.size() != size) {
      buffer->first_real_part_outputs_.resize(size);
    }
    if (buffer->first_imag_part_outputs_.size() != size) {
      buffer->first_imag_part_outputs_.resize(size);
    }
    if (buffer->second_real_part_outputs_.size() != size) {
      buffer->second_real_part_outputs_.resize(size);
    }
    if (buffer->second_imag_part_outputs_.size() != size) {
      buffer->second_imag_part_outputs_.resize(size);
    }
    if (real_part_output->GetNumRow() != fft_length_ ||
        real_part_output->GetNumColumn() != fft_length_) {
      real_part_output->Resize(fft_length_, fft_length_);
    }
    if (imag_part_output->GetNumRow() != fft_length_ ||
        imag_part_output->GetNumColumn() != fft_length_) {
      imag_part_output->Resize(fft_length_, fft_length_);
    }
  }

  double* x(&buffer->real_part_input_[0]);

  // First stage.
  std::fill(buffer->real_part_input_.begin() + num_row_,
            buffer->real_part_input_.end(), 0.0);
  for (int i(0); i < num_column_; ++i) {
    for (int j(0); j < num_row_; ++j) {
      x[j] = real_part_input[j][i];
    }
    if (!real_valued_fast_fourier_transform_.Run(
            buffer->real_part_input_, &buffer->first_real_part_outputs_[i],
            &buffer->first_imag_part_outputs_[i],
            &buffer->fast_fourier_transform_buffer_)) {
      return false;
    }
  }

  double* y(&buffer->imag_part_input_[0]);

  // Second stage.
  std::fill(buffer->real_part_input_.begin() + num_column_,
            buffer->real_part_input_.end(), 0.0);
  std::fill(buffer->imag_part_input_.begin() + num_column_,
            buffer->imag_part_input_.end(), 0.0);
  for (int i(0); i < fft_length_; ++i) {
    for (int j(0); j < num_column_; ++j) {
      x[j] = buffer->first_real_part_outputs_[j][i];
      y[j] = buffer->first_imag_part_outputs_[j][i];
    }
    if (!fast_fourier_transform_.Run(buffer->real_part_input_,
                                     buffer->imag_part_input_,
                                     &buffer->second_real_part_outputs_[i],
                                     &buffer->second_imag_part_outputs_[i])) {
      return false;
    }
  }

  // Save results.
  for (int i(0); i < fft_length_; ++i) {
    std::copy(buffer->second_real_part_outputs_[i].begin(),
              buffer->second_real_part_outputs_[i].end(),
              (*real_part_output)[i]);
    std::copy(buffer->second_imag_part_outputs_[i].begin(),
              buffer->second_imag_part_outputs_[i].end(),
              (*imag_part_output)[i]);
  }

  return true;
}

bool TwoDimensionalRealValuedFastFourierTransform::Run(
    Matrix* real_part, Matrix* imag_part,
    TwoDimensionalRealValuedFastFourierTransform::Buffer* buffer) const {
  if (NULL == real_part) return false;
  return Run(*real_part, real_part, imag_part, buffer);
}

}  // namespace sptk
