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

#include "SPTK/filter/all_pole_digital_filter.h"

#include <algorithm>  // std::fill
#include <cstddef>    // std::size_t

namespace sptk {

AllPoleDigitalFilter::AllPoleDigitalFilter(int num_filter_order,
                                           bool transposition)
    : num_filter_order_(num_filter_order),
      transposition_(transposition),
      is_valid_(true) {
  if (num_filter_order_ < 0) {
    is_valid_ = false;
    return;
  }
}

bool AllPoleDigitalFilter::Run(const std::vector<double>& filter_coefficients,
                               double filter_input, double* filter_output,
                               AllPoleDigitalFilter::Buffer* buffer) const {
  // Check inputs.
  if (!is_valid_ ||
      filter_coefficients.size() !=
          static_cast<std::size_t>(num_filter_order_ + 1) ||
      NULL == filter_output || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  if (buffer->d_.size() != static_cast<std::size_t>(num_filter_order_)) {
    buffer->d_.resize(num_filter_order_);
    std::fill(buffer->d_.begin(), buffer->d_.end(), 0.0);
  }

  const double gained_input(filter_input * filter_coefficients[0]);
  if (0 == num_filter_order_) {
    *filter_output = gained_input;
    return true;
  }

  const double* a(&(filter_coefficients[1]));
  double* d(&buffer->d_[0]);
  double sum(gained_input);

  // Apply all-pole filter.
  if (transposition_) {
    sum -= d[0];
    for (int m(1); m < num_filter_order_; ++m) {
      d[m - 1] = d[m] + a[m - 1] * sum;
    }
    d[num_filter_order_ - 1] = a[num_filter_order_ - 1] * sum;
  } else {
    for (int m(num_filter_order_ - 1); 0 < m; --m) {
      sum -= a[m] * d[m];
      d[m] = d[m - 1];
    }
    sum -= a[0] * d[0];
    d[0] = sum;
  }

  // Save result.
  *filter_output = sum;

  return true;
}

bool AllPoleDigitalFilter::Run(const std::vector<double>& filter_coefficients,
                               double* input_and_output,
                               AllPoleDigitalFilter::Buffer* buffer) const {
  return Run(filter_coefficients, *input_and_output, input_and_output, buffer);
}

}  // namespace sptk
