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

#include "SPTK/filter/mglsa_digital_filter.h"

#include <algorithm>  // std::fill
#include <cmath>      // std::exp
#include <cstddef>    // std::size_t

namespace sptk {

MglsaDigitalFilter::MglsaDigitalFilter(int num_filter_order, int num_pade_order,
                                       int num_stage, double alpha,
                                       bool transposition)
    : num_filter_order_(num_filter_order),
      num_stage_(num_stage),
      alpha_(alpha),
      transposition_(transposition),
      mlsa_digital_filter_(num_filter_order_, num_pade_order, alpha_,
                           transposition_),
      is_valid_(true) {
  if (num_filter_order_ < 0 || num_stage_ < 0 || !sptk::IsValidAlpha(alpha_)) {
    is_valid_ = false;
    return;
  }
  if (0 == num_stage && !mlsa_digital_filter_.IsValid()) {
    is_valid_ = false;
  }
}

bool MglsaDigitalFilter::Run(const std::vector<double>& filter_coefficients,
                             double filter_input, double* filter_output,
                             MglsaDigitalFilter::Buffer* buffer) const {
  // check inputs
  if (!is_valid_ ||
      filter_coefficients.size() !=
          static_cast<std::size_t>(num_filter_order_ + 1) ||
      NULL == filter_output || NULL == buffer) {
    return false;
  }

  if (0 == num_stage_) {
    return mlsa_digital_filter_.Run(filter_coefficients, filter_input,
                                    filter_output,
                                    &(buffer->mlsa_digital_filter_buffer_));
  }

  // prepare memories
  if (buffer->signals_.size() !=
      static_cast<std::size_t>((num_filter_order_ + 1) * num_stage_)) {
    buffer->signals_.resize((num_filter_order_ + 1) * num_stage_);
    std::fill(buffer->signals_.begin(), buffer->signals_.end(), 0.0);
  }

  const double gained_input(filter_input * std::exp(filter_coefficients[0]));
  if (0 == num_filter_order_) {
    *filter_output = gained_input;
    return true;
  }

  const double* b(&(filter_coefficients[1]));
  const double beta(1.0 - alpha_ * alpha_);
  double x(gained_input);

  for (int i(0); i < num_stage_; ++i) {
    double* d(&buffer->signals_[(num_filter_order_ + 1) * i]);
    if (transposition_) {
      x -= beta * d[0];
      d[num_filter_order_] =
          b[num_filter_order_ - 1] * x + alpha_ * d[num_filter_order_ - 1];
      for (int j(num_filter_order_ - 1); 0 < j; --j) {
        d[j] += b[j - 1] * x + alpha_ * (d[j - 1] - d[j + 1]);
      }

      for (int j(0); j < num_filter_order_; ++j) {
        d[j] = d[j + 1];
      }
    } else {
      double y(d[0] * b[0]);
      for (int j(1); j < num_filter_order_; ++j) {
        d[j] += alpha_ * (d[j + 1] - d[j - 1]);
        y += d[j] * b[j];
      }
      x -= y;

      for (int j(num_filter_order_); 0 < j; --j) {
        d[j] = d[j - 1];
      }
      d[0] = alpha_ * d[0] + beta * x;
    }
  }

  *filter_output = x;

  return true;
}

}  // namespace sptk
