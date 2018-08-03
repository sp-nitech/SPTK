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

#include "SPTK/math/discrete_fourier_transform.h"

#include <cmath>    // std::cos, std::sin
#include <cstddef>  // std::size_t

namespace sptk {

DiscreteFourierTransform::DiscreteFourierTransform(int dft_length)
    : dft_length_(dft_length), is_valid_(true) {
  if (dft_length_ <= 0) {
    is_valid_ = false;
  }
}

bool DiscreteFourierTransform::Run(
    const std::vector<double>& real_part_input,
    const std::vector<double>& imaginary_part_input,
    std::vector<double>* real_part_output,
    std::vector<double>* imaginary_part_output) const {
  // check inputs
  if (!is_valid_ ||
      real_part_input.size() != static_cast<std::size_t>(dft_length_) ||
      imaginary_part_input.size() != static_cast<std::size_t>(dft_length_) ||
      NULL == real_part_output || NULL == imaginary_part_output) {
    return false;
  }

  // prepare memories
  if (real_part_output->size() != static_cast<std::size_t>(dft_length_)) {
    real_part_output->resize(dft_length_);
  }
  if (imaginary_part_output->size() != static_cast<std::size_t>(dft_length_)) {
    imaginary_part_output->resize(dft_length_);
  }

  const double* input_x(&(real_part_input[0]));
  const double* input_y(&(imaginary_part_input[0]));
  double* output_x(&((*real_part_output)[0]));
  double* output_y(&((*imaginary_part_output)[0]));

  for (int k(0); k < dft_length_; ++k) {
    double sum_x(0.0);
    double sum_y(0.0);
    for (int n(0); n < dft_length_; ++n) {
      const double sin_w(std::sin(-2.0 * sptk::kPi * n * k / dft_length_));
      const double cos_w(std::cos(-2.0 * sptk::kPi * n * k / dft_length_));
      sum_x += input_x[n] * cos_w - input_y[n] * sin_w;
      sum_y += input_x[n] * sin_w + input_y[n] * cos_w;
    }
    output_x[k] = sum_x;
    output_y[k] = sum_y;
  }

  return true;
}

}  // namespace sptk
