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
//                1996-2019  Nagoya Institute of Technology          //
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

namespace sptk {

InfiniteImpulseResponseDigitalFilter::InfiniteImpulseResponseDigitalFilter(
    const std::vector<double>& denominator_filter_coefficients,
    const std::vector<double>& numerator_filter_coefficients)
    : denominator_filter_coefficients_(denominator_filter_coefficients),
      numerator_filter_coefficients_(numerator_filter_coefficients),
      num_denominator_filter_order_(
          static_cast<int>(denominator_filter_coefficients.size()) - 1),
      num_numerator_filter_order_(
          static_cast<int>(numerator_filter_coefficients.size()) - 1),
      num_filter_order_(
          std::max(num_denominator_filter_order_, num_numerator_filter_order_)),
      is_valid_(true) {
  if (num_denominator_filter_order_ < 0 || num_numerator_filter_order_ < 0) {
    is_valid_ = false;
  }
}

bool InfiniteImpulseResponseDigitalFilter::Run(
    double filter_input, double* filter_output,
    InfiniteImpulseResponseDigitalFilter::Buffer* buffer) const {
  // check inputs
  if (!is_valid_ || NULL == filter_output || NULL == buffer) {
    return false;
  }

  // prepare memory
  if (buffer->signals_.size() !=
      static_cast<std::size_t>(num_filter_order_ + 1)) {
    buffer->signals_.resize(num_filter_order_ + 1);
    std::fill(buffer->signals_.begin(), buffer->signals_.end(), 0.0);
  }

  double* d(&buffer->signals_[0]);

  {
    const double* a(&(denominator_filter_coefficients_[0]));
    double x(-filter_input * a[0]);
    for (int i(1), p(buffer->p_ - 1); i <= num_denominator_filter_order_;
         ++i, --p) {
      if (p < 0) p = num_filter_order_;
      x += d[p] * a[i];
    }
    d[buffer->p_] = -x;
  }

  {
    const double* b(&(numerator_filter_coefficients_[0]));
    double y(0.0);
    for (int i(0), p(buffer->p_); i <= num_numerator_filter_order_; ++i, --p) {
      if (p < 0) p = num_filter_order_;
      y += d[p] * b[i];
    }
    *filter_output = y;
  }

  if (num_filter_order_ < ++buffer->p_) {
    buffer->p_ = 0;
  }

  return true;
}

}  // namespace sptk
