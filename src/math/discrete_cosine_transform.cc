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
//                1996-2017  Nagoya Institute of Technology          //
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

#include "SPTK/math/discrete_cosine_transform.h"

#include <algorithm>  // std::copy, std::reverse_copy
#include <cmath>      // std::cos, std::sin, std::sqrt
#include <cstddef>    // std::size_t

namespace sptk {

DiscreteCosineTransform::DiscreteCosineTransform(int dct_length)
    : dct_length_(dct_length), fourier_transform_(2 * dct_length_) {
  if (!fourier_transform_.IsValid()) {
    return;
  }

  const int dft_length(2 * dct_length_);
  const double argument(sptk::kPi / dft_length);
  const double c(1.0 / std::sqrt(dft_length));
  cosine_table_.resize(dct_length_);
  sine_table_.resize(dct_length_);
  for (int i(0); i < dct_length_; ++i) {
    cosine_table_[i] = std::cos(argument * i) * c;
    sine_table_[i] = -std::sin(argument * i) * c;
  }
  cosine_table_[0] /= std::sqrt(2.0);
  sine_table_[0] /= std::sqrt(2.0);
}

bool DiscreteCosineTransform::Run(
    const std::vector<double>& real_part_input,
    const std::vector<double>& imaginary_part_input,
    std::vector<double>* real_part_output,
    std::vector<double>* imaginary_part_output,
    DiscreteCosineTransform::Buffer* buffer) const {
  // check inputs
  if (!fourier_transform_.IsValid() ||
      real_part_input.size() != static_cast<std::size_t>(dct_length_) ||
      imaginary_part_input.size() != static_cast<std::size_t>(dct_length_) ||
      NULL == real_part_output || NULL == imaginary_part_output ||
      NULL == buffer) {
    return false;
  }

  if (buffer->fourier_transform_real_part_input_.size() <
      static_cast<std::size_t>(2 * dct_length_)) {
    buffer->fourier_transform_real_part_input_.resize(2 * dct_length_);
  }
  if (buffer->fourier_transform_imaginary_part_input_.size() <
      static_cast<std::size_t>(2 * dct_length_)) {
    buffer->fourier_transform_imaginary_part_input_.resize(2 * dct_length_);
  }
  if (real_part_output->size() < static_cast<std::size_t>(dct_length_)) {
    real_part_output->resize(dct_length_);
  }
  if (imaginary_part_output->size() < static_cast<std::size_t>(dct_length_)) {
    imaginary_part_output->resize(dct_length_);
  }

  std::copy(real_part_input.begin(), real_part_input.end(),
            buffer->fourier_transform_real_part_input_.begin());
  std::reverse_copy(
      real_part_input.begin(), real_part_input.end(),
      buffer->fourier_transform_real_part_input_.end() - dct_length_);

  std::copy(imaginary_part_input.begin(), imaginary_part_input.end(),
            buffer->fourier_transform_imaginary_part_input_.begin());
  std::reverse_copy(
      imaginary_part_input.begin(), imaginary_part_input.end(),
      buffer->fourier_transform_imaginary_part_input_.end() - dct_length_);

  if (!fourier_transform_.Run(
          buffer->fourier_transform_real_part_input_,
          buffer->fourier_transform_imaginary_part_input_,
          &buffer->fourier_transform_real_part_output_,
          &buffer->fourier_transform_imaginary_part_output_)) {
    return false;
  }

  double* discrete_cosine_transform_real_part_output(&((*real_part_output)[0]));
  double* discrete_cosine_transform_imaginary_part_output(
      &((*imaginary_part_output)[0]));
  double* fourier_transform_real_part_output(
      &buffer->fourier_transform_real_part_output_[0]);
  double* fourier_transform_imaginary_part_output(
      &buffer->fourier_transform_imaginary_part_output_[0]);

  for (int i(0); i < dct_length_; ++i) {
    discrete_cosine_transform_real_part_output[i] =
        fourier_transform_real_part_output[i] * cosine_table_[i] -
        fourier_transform_imaginary_part_output[i] * sine_table_[i];
    discrete_cosine_transform_imaginary_part_output[i] =
        fourier_transform_real_part_output[i] * sine_table_[i] +
        fourier_transform_imaginary_part_output[i] * cosine_table_[i];
  }

  return true;
}

}  // namespace sptk
