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

#include "SPTK/filter/infinite_impulse_response_digital_filter.h"

#include <algorithm>  // std::fill, std::max
#include <cstddef>    // std::size_t

namespace {

void MovePointer(int max, int* p) {
  --(*p);
  if (*p < 0) *p = max;
}

}  // namespace

namespace sptk {

InfiniteImpulseResponseDigitalFilter::InfiniteImpulseResponseDigitalFilter(
    const std::vector<double>& denominator_coefficients,
    const std::vector<double>& numerator_coefficients)
    : denominator_coefficients_(denominator_coefficients),
      numerator_coefficients_(numerator_coefficients),
      num_denominator_order_(static_cast<int>(denominator_coefficients.size()) -
                             1),
      num_numerator_order_(static_cast<int>(numerator_coefficients.size()) - 1),
      num_filter_order_(std::max(num_denominator_order_, num_numerator_order_)),
      is_valid_(true) {
  if (num_denominator_order_ < 0 || num_numerator_order_ < 0) {
    is_valid_ = false;
    return;
  }
}

bool InfiniteImpulseResponseDigitalFilter::Run(
    double input, double* output,
    InfiniteImpulseResponseDigitalFilter::Buffer* buffer) const {
  // Check inputs.
  if (!is_valid_ || NULL == output || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  if (buffer->d_.size() != static_cast<std::size_t>(num_filter_order_ + 1)) {
    buffer->d_.resize(num_filter_order_ + 1);
    std::fill(buffer->d_.begin(), buffer->d_.end(), 0.0);
    buffer->p_ = 0;
  }

  double* d(&buffer->d_[0]);

  {
    const double* a(&(denominator_coefficients_[0]));
    double x(-input * a[0]);
    for (int i(1), p(buffer->p_); i <= num_denominator_order_; ++i) {
      MovePointer(num_filter_order_, &p);
      x += d[p] * a[i];
    }
    d[buffer->p_] = -x;
  }

  {
    const double* b(&(numerator_coefficients_[0]));
    double y(0.0);
    for (int i(0), p(buffer->p_ + 1); i <= num_numerator_order_; ++i) {
      MovePointer(num_filter_order_, &p);
      y += d[p] * b[i];
    }
    *output = y;
  }

  // Update pointer of ring buffer.
  ++(buffer->p_);
  if (num_filter_order_ < buffer->p_) {
    buffer->p_ = 0;
  }

  return true;
}

bool InfiniteImpulseResponseDigitalFilter::Run(
    double* input_and_output,
    InfiniteImpulseResponseDigitalFilter::Buffer* buffer) const {
  if (NULL == input_and_output) return false;
  return Run(*input_and_output, input_and_output, buffer);
}

}  // namespace sptk
