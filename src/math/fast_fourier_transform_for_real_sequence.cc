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

#include "SPTK/math/fast_fourier_transform_for_real_sequence.h"

#include <algorithm>  // std::fill
#include <cmath>      // std::sin
#include <cstddef>    // std::size_t

namespace sptk {

FastFourierTransformForRealSequence::FastFourierTransformForRealSequence(
    int num_order, int fft_length)
    : num_order_(num_order),
      fft_length_(fft_length),
      half_fft_length_(fft_length / 2),
      fast_fourier_transform_(half_fft_length_ - 1, half_fft_length_),
      is_valid_(true) {
  if (!fast_fourier_transform_.IsValid() || !IsPowerOfTwo(fft_length) ||
      fft_length <= num_order || num_order < 0) {
    is_valid_ = false;
    return;
  }

  const int table_size(fft_length - fft_length / 4 + 1);
  const double argument(sptk::kPi / fft_length * 2);
  sine_table_.resize(table_size);
  for (int i(0); i < table_size; ++i) {
    sine_table_[i] = std::sin(argument * i);
  }
  sine_table_[fft_length / 2] = 0.0;
}

bool FastFourierTransformForRealSequence::Run(
    const std::vector<double>& real_part_input,
    std::vector<double>* real_part_output,
    std::vector<double>* imaginary_part_output,
    FastFourierTransformForRealSequence::Buffer* buffer) const {
  // check inputs
  if (!is_valid_ ||
      real_part_input.size() != static_cast<std::size_t>(num_order_ + 1) ||
      NULL == real_part_output || NULL == imaginary_part_output ||
      NULL == buffer) {
    return false;
  }

  // prepare memories
  if (buffer->real_part_input_.size() !=
      static_cast<std::size_t>(half_fft_length_)) {
    buffer->real_part_input_.resize(half_fft_length_);
  }
  if (buffer->imaginary_part_input_.size() !=
      static_cast<std::size_t>(half_fft_length_)) {
    buffer->imaginary_part_input_.resize(half_fft_length_);
  }
  if (real_part_output->size() < static_cast<std::size_t>(fft_length_)) {
    real_part_output->resize(fft_length_);
  }
  if (imaginary_part_output->size() < static_cast<std::size_t>(fft_length_)) {
    imaginary_part_output->resize(fft_length_);
  }

  // get values and fill zero
  const int input_length(num_order_ + 1);
  for (int i(0), j(0); i < input_length; ++j) {
    buffer->real_part_input_[j] = real_part_input[i++];
    if (input_length <= i) break;
    buffer->imaginary_part_input_[j] = real_part_input[i++];
  }
  std::fill(buffer->real_part_input_.begin() + (input_length + 1) / 2,
            buffer->real_part_input_.end(), 0.0);
  std::fill(buffer->imaginary_part_input_.begin() + input_length / 2,
            buffer->imaginary_part_input_.end(), 0.0);

  // run fast Fourier transform
  if (!fast_fourier_transform_.Run(buffer->real_part_input_,
                                   buffer->imaginary_part_input_,
                                   real_part_output, imaginary_part_output)) {
    return false;
  }

  double* x(&((*real_part_output)[0]));
  double* y(&((*imaginary_part_output)[0]));
  double* xp(x);
  double* yp(y);
  double* xq(xp + fft_length_);
  double* yq(yp + fft_length_);
  *(xp + half_fft_length_) = *xp - *yp;
  *xp = *xp + *yp;
  *(yp + half_fft_length_) = 0;
  *yp = 0;

  double* sinp(const_cast<double*>(&(sine_table_[0])));
  double* cosp(const_cast<double*>(&(sine_table_[0])) + fft_length_ / 4);
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

}  // namespace sptk
