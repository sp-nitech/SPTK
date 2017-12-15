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

#include "SPTK/filter/line_spectral_pairs_digital_filter.h"

#include <algorithm>  // std::fill
#include <cmath>      // std::cos
#include <cstddef>    // std::size_t

namespace sptk {

bool LineSpectralPairsDigitalFilter::Run(
    const std::vector<double>& filter_coefficients, double filter_input,
    double* filter_output,
    LineSpectralPairsDigitalFilter::Buffer* buffer) const {
  // check inputs
  if (!is_valid_ ||
      filter_coefficients.size() !=
          static_cast<std::size_t>(num_filter_order_ + 1) ||
      NULL == filter_output || NULL == buffer) {
    return false;
  }

  // prepare memory
  if (buffer->signals1_.size() !=
      static_cast<std::size_t>(num_filter_order_ + 1)) {
    buffer->signals1_.resize(num_filter_order_ + 1);
    std::fill(buffer->signals1_.begin(), buffer->signals1_.end(), 0.0);
  }
  if (buffer->signals2_.size() !=
      static_cast<std::size_t>(num_filter_order_ + 1)) {
    buffer->signals2_.resize(num_filter_order_ + 1);
    std::fill(buffer->signals2_.begin(), buffer->signals2_.end(), 0.0);
  }

  // set value
  const double gained_input(filter_input * filter_coefficients[0]);
  if (0 == num_filter_order_) {
    *filter_output = gained_input;
    return true;
  }

  // get values
  const double* coefficients(&(filter_coefficients[0]));
  double* signals1(&buffer->signals1_[0]);
  double* signals2(&buffer->signals2_[0]);

  // apply filter
  double sum(gained_input);
  {
    double x1(signals1[0]);
    double x2(signals2[0]);
    for (int i(1); i < num_filter_order_; i += 2) {
      signals1[i] -= 2.0 * x1 * std::cos(coefficients[i]);
      signals2[i] -= 2.0 * x2 * std::cos(coefficients[i + 1]);
      signals1[i + 1] += x1;
      signals2[i + 1] += x2;
      x1 = signals1[i + 1];
      x2 = signals2[i + 1];
      sum += signals1[i] + signals2[i];
    }
    if (1 == num_filter_order_ % 2) {
      signals1[num_filter_order_] -=
          2.0 * x1 * std::cos(coefficients[num_filter_order_]);
    }
    sum += signals1[num_filter_order_] - signals2[num_filter_order_];
  }

  // save result
  *filter_output = sum;

  // shift stored signals
  for (int i(num_filter_order_); 0 < i; --i) {
    signals1[i] = signals1[i - 1];
    signals2[i] = signals2[i - 1];
  }
  const double delayed_output(-0.5 * sum);
  signals1[0] = delayed_output;
  signals2[0] = delayed_output;

  return true;
}

}  // namespace sptk
