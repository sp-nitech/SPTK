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

#include "SPTK/filter/all_pole_lattice_digital_filter.h"

#include <algorithm>  // std::fill
#include <cstddef>    // std::size_t

namespace sptk {

bool AllPoleLatticeDigitalFilter::Run(
    const std::vector<double>& filter_coefficients, double filter_input,
    double* filter_output,
    AllPoleLatticeDigitalFilter::StoredSignals* stored_signals) const {
  // check inputs
  if (filter_coefficients.size() !=
          static_cast<std::size_t>(num_filter_order_ + 1) ||
      NULL == filter_output || NULL == stored_signals || !is_valid_) {
    return false;
  }

  // prepare memory
  if (stored_signals->signals_.size() !=
      static_cast<std::size_t>(num_filter_order_)) {
    stored_signals->signals_.resize(num_filter_order_);
    std::fill(stored_signals->signals_.begin(), stored_signals->signals_.end(),
              0.0);
  }

  // set value
  const double gained_input(filter_input * filter_coefficients[0]);
  if (0 == num_filter_order_) {
    *filter_output = gained_input;
    return true;
  }

  // get values
  const double* coefficients(&(filter_coefficients[0]));
  double* signals(&stored_signals->signals_[0]);

  // apply filter
  double sum(gained_input);
  sum -= coefficients[num_filter_order_] * signals[num_filter_order_ - 1];
  for (int i(num_filter_order_ - 1); 0 < i; --i) {
    sum -= coefficients[i] * signals[i - 1];
    signals[i] = signals[i - 1] + coefficients[i] * sum;
  }
  signals[0] = sum;

  // save result
  *filter_output = sum;

  return true;
}

}  // namespace sptk
