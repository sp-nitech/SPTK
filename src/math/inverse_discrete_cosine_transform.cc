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
//                1996-2018  Nagoya Institute of Technology          //
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

#include "SPTK/math/inverse_discrete_cosine_transform.h"

#include <algorithm>   // std::copy, std::reverse, std::transform
#include <cmath>       // std::cos, std::sin, std::sqrt
#include <cstddef>     // std::size_t
#include <functional>  // std::bind1st, std::multiplies

namespace sptk {

InverseDiscreteCosineTransform::InverseDiscreteCosineTransform(int dct_length)
    : dct_length_(dct_length), fourier_transform_(2 * dct_length) {
  if (!fourier_transform_.IsValid()) {
    return;
  }

  const int dft_length(2 * dct_length_);
  const double argument(sptk::kPi / dft_length);
  const double c(1.0 / std::sqrt(dft_length));
  cosine_table_.resize(dft_length);
  sine_table_.resize(dft_length);
  cosine_table_[0] = c * std::sqrt(2.0);
  sine_table_[0] = 0.0;
  for (int i(1); i < dft_length; ++i) {
    cosine_table_[i] = std::cos(argument * i) * c;
    sine_table_[i] = -std::sin(argument * i) * c;
  }
}

bool InverseDiscreteCosineTransform::Run(
    const std::vector<double>& real_part_input,
    const std::vector<double>& imaginary_part_input,
    std::vector<double>* real_part_output,
    std::vector<double>* imaginary_part_output,
    InverseDiscreteCosineTransform::Buffer* buffer) const {
  // check inputs
  if (!fourier_transform_.IsValid() ||
      real_part_input.size() != static_cast<std::size_t>(dct_length_) ||
      imaginary_part_input.size() != static_cast<std::size_t>(dct_length_) ||
      NULL == real_part_output || NULL == imaginary_part_output ||
      NULL == buffer) {
    return false;
  }

  // prepare buffer
  const int dft_length(2 * dct_length_);
  if (buffer->fourier_transform_real_part_input_.size() !=
      static_cast<std::size_t>(dft_length)) {
    buffer->fourier_transform_real_part_input_.resize(dft_length);
  }
  if (buffer->fourier_transform_imaginary_part_input_.size() !=
      static_cast<std::size_t>(dft_length)) {
    buffer->fourier_transform_imaginary_part_input_.resize(dft_length);
  }

  // fill buffer for real part
  std::copy(real_part_input.begin(), real_part_input.end(),
            buffer->fourier_transform_real_part_input_.begin());
  std::transform(
      real_part_input.begin() + 1, real_part_input.end(),
      buffer->fourier_transform_real_part_input_.begin() + dct_length_ + 1,
      std::bind1st(std::multiplies<double>(), -1.0));
  std::reverse(
      buffer->fourier_transform_real_part_input_.begin() + dct_length_ + 1,
      buffer->fourier_transform_real_part_input_.end());
  buffer->fourier_transform_real_part_input_[dct_length_] = 0.0;

  // fill buffer for imaginary part
  std::copy(imaginary_part_input.begin(), imaginary_part_input.end(),
            buffer->fourier_transform_imaginary_part_input_.begin());
  std::transform(
      imaginary_part_input.begin() + 1, imaginary_part_input.end(),
      buffer->fourier_transform_imaginary_part_input_.begin() + dct_length_ + 1,
      std::bind1st(std::multiplies<double>(), -1.0));
  std::reverse(
      buffer->fourier_transform_imaginary_part_input_.begin() + dct_length_ + 1,
      buffer->fourier_transform_imaginary_part_input_.end());
  buffer->fourier_transform_imaginary_part_input_[dct_length_] = 0.0;

  // make input for fourier transform using sine and cosine table
  {
    double* fourier_transform_real_part_input(
        &buffer->fourier_transform_real_part_input_[0]);
    double* fourier_transform_imaginary_part_input(
        &buffer->fourier_transform_imaginary_part_input_[0]);

    for (int i(0); i < dft_length; ++i) {
      const double temporary_real_part(fourier_transform_real_part_input[i]);
      const double temporary_imaginary_part(
          fourier_transform_imaginary_part_input[i]);
      fourier_transform_real_part_input[i] =
          temporary_real_part * cosine_table_[i] -
          temporary_imaginary_part * sine_table_[i];
      fourier_transform_imaginary_part_input[i] =
          temporary_real_part * sine_table_[i] +
          temporary_imaginary_part * cosine_table_[i];
    }
  }

  if (!fourier_transform_.Run(buffer->fourier_transform_real_part_input_,
                              buffer->fourier_transform_imaginary_part_input_,
                              real_part_output, imaginary_part_output)) {
    return false;
  }

  real_part_output->resize(dct_length_);
  imaginary_part_output->resize(dct_length_);

  return true;
}

}  // namespace sptk
