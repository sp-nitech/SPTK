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

#include "SPTK/converter/linear_predictive_coefficients_to_cepstrum.h"

#include <cmath>    // std::log
#include <cstddef>  // std::size_t

namespace sptk {

LinearPredictiveCoefficientsToCepstrum::LinearPredictiveCoefficientsToCepstrum(
    int num_input_order, int num_output_order)
    : num_input_order_(num_input_order),
      num_output_order_(num_output_order),
      is_valid_(true) {
  if (num_input_order_ < 0 || num_output_order_ < 0) {
    is_valid_ = false;
  }
}

bool LinearPredictiveCoefficientsToCepstrum::Run(
    const std::vector<double>& linear_predictive_coefficients,
    std::vector<double>* cepstrum) const {
  // check inputs
  if (!is_valid_ ||
      linear_predictive_coefficients.size() !=
          static_cast<std::size_t>(num_input_order_ + 1) ||
      NULL == cepstrum) {
    return false;
  }

  // prepare memory
  const int output_length(num_output_order_ + 1);
  if (cepstrum->size() < static_cast<std::size_t>(output_length)) {
    cepstrum->resize(output_length);
  }

  // get values
  const double* input(&(linear_predictive_coefficients[0]));
  double* output(&((*cepstrum)[0]));

  output[0] = std::log(input[0]);
  if (0 == num_output_order_) return true;

  output[1] = -input[1];

  for (int n(2); n <= num_output_order_; ++n) {
    double sum(0.0);
    const int k_first((num_input_order_ < n) ? (n - num_input_order_) : 1);
    for (int k(k_first); k < n; ++k) {
      sum -= k * output[k] * input[n - k];
    }
    output[n] = sum / n;

    if (n <= num_input_order_) {
      output[n] -= input[n];
    }
  }

  return true;
}

}  // namespace sptk
