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

#include "SPTK/converter/linear_predictive_coefficients_to_parcor_coefficients.h"

#include <algorithm>   // std::copy, std::fill, std::transform
#include <cmath>       // std::fabs
#include <cstddef>     // std::size_t
#include <functional>  // std::bind1st, std::multiplies

namespace sptk {

LinearPredictiveCoefficientsToParcorCoefficients::
    LinearPredictiveCoefficientsToParcorCoefficients(int num_order,
                                                     double gamma)
    : num_order_(num_order), gamma_(gamma), is_valid_(true) {
  if (num_order_ < 0) {
    is_valid_ = false;
  }
}

bool LinearPredictiveCoefficientsToParcorCoefficients::Run(
    const std::vector<double>& linear_predictive_coefficients,
    std::vector<double>* parcor_coefficients, bool* is_stable,
    LinearPredictiveCoefficientsToParcorCoefficients::Buffer* buffer) const {
  // check inputs
  if (!is_valid_ ||
      linear_predictive_coefficients.size() !=
          static_cast<std::size_t>(num_order_ + 1) ||
      NULL == parcor_coefficients || NULL == buffer || NULL == is_stable) {
    return false;
  }

  // prepare memory
  const int output_length(num_order_ + 1);
  if (parcor_coefficients->size() != static_cast<std::size_t>(output_length)) {
    parcor_coefficients->resize(output_length);
  }

  // set value
  *is_stable = true;

  (*parcor_coefficients)[0] = linear_predictive_coefficients[0];
  if (0 == num_order_) {
    return true;
  }
  if (0.0 == gamma_) {
    std::fill(parcor_coefficients->begin() + 1, parcor_coefficients->end(),
              0.0);
    return true;
  }

  // prepare buffer
  if (buffer->a_.size() != static_cast<std::size_t>(output_length)) {
    buffer->a_.resize(output_length);
  }

  // get value
  double* output(&((*parcor_coefficients)[0]));

  // transform linear predictive coefficients to parcor coefficients
  if (1.0 == gamma_) {
    std::copy(linear_predictive_coefficients.begin(),
              linear_predictive_coefficients.end(), buffer->a_.begin());
  } else {
    buffer->a_[0] = linear_predictive_coefficients[0];
    std::transform(linear_predictive_coefficients.begin() + 1,
                   linear_predictive_coefficients.end(), buffer->a_.begin() + 1,
                   std::bind1st(std::multiplies<double>(), gamma_));
  }

  double* a(&buffer->a_[0]);
  for (int i(num_order_); 1 <= i; --i) {
    for (int j(1); j <= i; ++j) {
      output[j] = a[j];
    }
    const double denominator(1.0 - output[i] * output[i]);
    if (0.0 == denominator) {
      return false;
    }
    if (1.0 <= std::fabs(output[i])) {
      *is_stable = false;
    }
    for (int j(1); j < i; ++j) {
      a[j] = (output[j] - output[i] * output[i - j]) / denominator;
    }
  }

  return true;
}

}  // namespace sptk
