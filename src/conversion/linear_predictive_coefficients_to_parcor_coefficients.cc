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

#include "SPTK/conversion/linear_predictive_coefficients_to_parcor_coefficients.h"

#include <algorithm>  // std::copy, std::transform
#include <cmath>      // std::fabs
#include <cstddef>    // std::size_t

namespace sptk {

LinearPredictiveCoefficientsToParcorCoefficients::
    LinearPredictiveCoefficientsToParcorCoefficients(int num_order,
                                                     double gamma)
    : num_order_(num_order), gamma_(gamma), is_valid_(true) {
  if (num_order_ < 0 || !sptk::IsValidGamma(gamma_)) {
    is_valid_ = false;
    return;
  }
}

bool LinearPredictiveCoefficientsToParcorCoefficients::Run(
    const std::vector<double>& linear_predictive_coefficients,
    std::vector<double>* parcor_coefficients, bool* is_stable,
    LinearPredictiveCoefficientsToParcorCoefficients::Buffer* buffer) const {
  // Check inputs.
  const int length(num_order_ + 1);
  if (!is_valid_ ||
      linear_predictive_coefficients.size() !=
          static_cast<std::size_t>(length) ||
      NULL == parcor_coefficients || NULL == is_stable || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  if (parcor_coefficients->size() != static_cast<std::size_t>(length)) {
    parcor_coefficients->resize(length);
  }
  if (buffer->a_.size() != static_cast<std::size_t>(length)) {
    buffer->a_.resize(length);
  }

  *is_stable = true;

  // Copy gain.
  (*parcor_coefficients)[0] = linear_predictive_coefficients[0];
  if (0 == num_order_) {
    return true;
  }

  // Set initial condition.
  if (1.0 == gamma_) {
    std::copy(linear_predictive_coefficients.begin(),
              linear_predictive_coefficients.end(), buffer->a_.begin());
  } else {
    const double gamma(gamma_);
    std::transform(linear_predictive_coefficients.begin() + 1,
                   linear_predictive_coefficients.end(), buffer->a_.begin() + 1,
                   [gamma](double x) { return x * gamma; });
  }

  // Apply recursive formula.
  double* a(&buffer->a_[0]);
  double* k(&((*parcor_coefficients)[0]));
  for (int i(num_order_); 1 <= i; --i) {
    for (int m(1); m <= i; ++m) {
      k[m] = a[m];
    }
    const double denominator(1.0 - k[i] * k[i]);
    if (0.0 == denominator) {
      return false;
    }
    if (1.0 <= std::fabs(k[i])) {
      *is_stable = false;
    }
    for (int m(1); m < i; ++m) {
      a[m] = (k[m] - k[i] * k[i - m]) / denominator;
    }
  }

  return true;
}

bool LinearPredictiveCoefficientsToParcorCoefficients::Run(
    std::vector<double>* input_and_output, bool* is_stable,
    LinearPredictiveCoefficientsToParcorCoefficients::Buffer* buffer) const {
  if (NULL == input_and_output) return false;
  return Run(*input_and_output, input_and_output, is_stable, buffer);
}

}  // namespace sptk
