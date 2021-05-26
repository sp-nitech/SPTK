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
//                1996-2020  Nagoya Institute of Technology          //
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

#include "SPTK/math/two_dimensional_inverse_fast_fourier_transform.h"

#include <algorithm>  // std::copy, std::fill
#include <cstddef>    // std::size_t

namespace sptk {

TwoDimensionalInverseFastFourierTransform::
    TwoDimensionalInverseFastFourierTransform(int num_row, int num_column,
                                              int fft_length)
    : num_row_(num_row),
      num_column_(num_column),
      fft_length_(fft_length),
      inverse_fast_fourier_transform_(fft_length_),
      is_valid_(true) {
  if (num_row_ <= 0 || fft_length_ < num_row_ || num_column_ <= 0 ||
      fft_length < num_column_ || !inverse_fast_fourier_transform_.IsValid()) {
    is_valid_ = false;
    return;
  }
}

bool TwoDimensionalInverseFastFourierTransform::Run(
    const Matrix& real_part_input, const Matrix& imag_part_input,
    Matrix* real_part_output, Matrix* imag_part_output,
    TwoDimensionalInverseFastFourierTransform::Buffer* buffer) const {
  // Check inputs.
  if (!is_valid_ || real_part_input.GetNumRow() != num_row_ ||
      real_part_input.GetNumColumn() != num_column_ ||
      imag_part_input.GetNumRow() != num_row_ ||
      imag_part_input.GetNumColumn() != num_column_ ||
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
  double* y(&buffer->imag_part_input_[0]);

  // First stage.
  std::fill(buffer->real_part_input_.begin() + num_row_,
            buffer->real_part_input_.end(), 0.0);
  std::fill(buffer->imag_part_input_.begin() + num_row_,
            buffer->imag_part_input_.end(), 0.0);
  for (int i(0); i < num_column_; ++i) {
    for (int j(0); j < num_row_; ++j) {
      x[j] = real_part_input[j][i];
      y[j] = imag_part_input[j][i];
    }
    if (!inverse_fast_fourier_transform_.Run(
            buffer->real_part_input_, buffer->imag_part_input_,
            &buffer->first_real_part_outputs_[i],
            &buffer->first_imag_part_outputs_[i])) {
      return false;
    }
  }

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
    if (!inverse_fast_fourier_transform_.Run(
            buffer->real_part_input_, buffer->imag_part_input_,
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

bool TwoDimensionalInverseFastFourierTransform::Run(
    Matrix* real_part, Matrix* imag_part,
    TwoDimensionalInverseFastFourierTransform::Buffer* buffer) const {
  if (NULL == real_part || NULL == imag_part) return false;
  return Run(*real_part, *imag_part, real_part, imag_part, buffer);
}

}  // namespace sptk
