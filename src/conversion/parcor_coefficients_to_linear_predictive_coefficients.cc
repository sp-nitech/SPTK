// ------------------------------------------------------------------------ //
// Copyright 2021 SPTK Working Group                                        //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ------------------------------------------------------------------------ //

#include "SPTK/conversion/parcor_coefficients_to_linear_predictive_coefficients.h"

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
