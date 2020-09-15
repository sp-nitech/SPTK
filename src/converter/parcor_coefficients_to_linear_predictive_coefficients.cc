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

#include "SPTK/converter/parcor_coefficients_to_linear_predictive_coefficients.h"

#include <algorithm>  // std::copy
#include <cstddef>    // std::size_t

namespace sptk {

ParcorCoefficientsToLinearPredictiveCoefficients::
    ParcorCoefficientsToLinearPredictiveCoefficients(int num_order)
    : num_order_(num_order), is_valid_(true) {
  if (num_order_ < 0) {
    is_valid_ = false;
    return;
  }
}

bool ParcorCoefficientsToLinearPredictiveCoefficients::Run(
    const std::vector<double>& parcor_coefficients,
    std::vector<double>* linear_predictive_coefficients,
    ParcorCoefficientsToLinearPredictiveCoefficients::Buffer* buffer) const {
  // Check inputs.
  const int length(num_order_ + 1);
  if (!is_valid_ ||
      parcor_coefficients.size() != static_cast<std::size_t>(length) ||
      NULL == linear_predictive_coefficients || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  if (linear_predictive_coefficients->size() !=
      static_cast<std::size_t>(length)) {
    linear_predictive_coefficients->resize(length);
  }
  if (buffer->a_.size() != static_cast<std::size_t>(num_order_)) {
    buffer->a_.resize(num_order_);
  }

  // Copy gain.
  (*linear_predictive_coefficients)[0] = parcor_coefficients[0];
  if (0 == num_order_) {
    return true;
  }

  // Set initial condition.
  std::copy(parcor_coefficients.begin(), parcor_coefficients.end() - 1,
            buffer->a_.begin());

  // Apply recursive formula.
  const double* k(&(parcor_coefficients[0]));
  double* prev_a(&buffer->a_[0]);
  double* a(&((*linear_predictive_coefficients)[0]));
  for (int i(2); i <= num_order_; ++i) {
    for (int m(1); m < i; ++m) {
      a[m] = prev_a[m] + k[i] * prev_a[i - m];
    }
    for (int m(1); m < i; ++m) {
      prev_a[m] = a[m];
    }
  }
  a[num_order_] = k[num_order_];

  return true;
}

bool ParcorCoefficientsToLinearPredictiveCoefficients::Run(
    std::vector<double>* input_and_output,
    ParcorCoefficientsToLinearPredictiveCoefficients::Buffer* buffer) const {
  if (NULL == input_and_output) return false;
  return Run(*input_and_output, input_and_output, buffer);
}

}  // namespace sptk
