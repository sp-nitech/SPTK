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

#include "SPTK/math/inverse_discrete_cosine_transform.h"

#include <algorithm>   // std::copy, std::reverse, std::transform
#include <cmath>       // std::cos, std::sin, std::sqrt
#include <cstddef>     // std::size_t
#include <functional>  // std::negate

namespace sptk {

InverseDiscreteCosineTransform::InverseDiscreteCosineTransform(int dct_length)
    : dct_length_(dct_length), fourier_transform_(2 * dct_length) {
  if (!fourier_transform_.IsValid()) {
    return;
  }

  const int dft_length(fourier_transform_.GetLength());
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
    const std::vector<double>& imag_part_input,
    std::vector<double>* real_part_output,
    std::vector<double>* imag_part_output,
    InverseDiscreteCosineTransform::Buffer* buffer) const {
  // Check inputs.
  if (!fourier_transform_.IsValid() ||
      real_part_input.size() != static_cast<std::size_t>(dct_length_) ||
      imag_part_input.size() != static_cast<std::size_t>(dct_length_) ||
      NULL == real_part_output || NULL == imag_part_output || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  const int dft_length(fourier_transform_.GetLength());
  if (buffer->fourier_transform_real_part_.size() !=
      static_cast<std::size_t>(dft_length)) {
    buffer->fourier_transform_real_part_.resize(dft_length);
  }
  if (buffer->fourier_transform_imag_part_.size() !=
      static_cast<std::size_t>(dft_length)) {
    buffer->fourier_transform_imag_part_.resize(dft_length);
  }

  // Prepare real part input.
  std::copy(real_part_input.begin(), real_part_input.end(),
            buffer->fourier_transform_real_part_.begin());
  std::transform(real_part_input.begin() + 1, real_part_input.end(),
                 buffer->fourier_transform_real_part_.begin() + dct_length_ + 1,
                 std::negate<double>());
  std::reverse(buffer->fourier_transform_real_part_.begin() + dct_length_ + 1,
               buffer->fourier_transform_real_part_.end());
  buffer->fourier_transform_real_part_[dct_length_] = 0.0;

  // Prepare imaginary part input.
  std::copy(imag_part_input.begin(), imag_part_input.end(),
            buffer->fourier_transform_imag_part_.begin());
  std::transform(imag_part_input.begin() + 1, imag_part_input.end(),
                 buffer->fourier_transform_imag_part_.begin() + dct_length_ + 1,
                 std::negate<double>());
  std::reverse(buffer->fourier_transform_imag_part_.begin() + dct_length_ + 1,
               buffer->fourier_transform_imag_part_.end());
  buffer->fourier_transform_imag_part_[dct_length_] = 0.0;

  const double* cosine_table(&(cosine_table_[0]));
  const double* sine_table(&(sine_table_[0]));
  double* fourier_transform_real_part(&buffer->fourier_transform_real_part_[0]);
  double* fourier_transform_imag_part(&buffer->fourier_transform_imag_part_[0]);

  for (int i(0); i < dft_length; ++i) {
    const double temp_real_part(fourier_transform_real_part[i]);
    const double temp_imag_part(fourier_transform_imag_part[i]);
    fourier_transform_real_part[i] =
        temp_real_part * cosine_table[i] - temp_imag_part * sine_table[i];
    fourier_transform_imag_part[i] =
        temp_real_part * sine_table[i] + temp_imag_part * cosine_table[i];
  }

  if (!fourier_transform_.Run(buffer->fourier_transform_real_part_,
                              buffer->fourier_transform_imag_part_,
                              real_part_output, imag_part_output)) {
    return false;
  }

  real_part_output->resize(dct_length_);
  imag_part_output->resize(dct_length_);

  return true;
}

bool InverseDiscreteCosineTransform::Run(
    std::vector<double>* real_part, std::vector<double>* imag_part,
    InverseDiscreteCosineTransform::Buffer* buffer) const {
  if (NULL == real_part || NULL == imag_part) return false;
  return Run(*real_part, *imag_part, real_part, imag_part, buffer);
}

}  // namespace sptk
