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

#include "SPTK/math/levinson_durbin_recursion.h"

#include <cmath>    // std::fabs, std::isnan, std::sqrt
#include <cstddef>  // std::size_t

namespace sptk {

LevinsonDurbinRecursion::LevinsonDurbinRecursion(int num_order)
    : num_order_(num_order), is_valid_(true) {
  if (num_order_ < 0) {
    is_valid_ = false;
    return;
  }
}

bool LevinsonDurbinRecursion::Run(
    const std::vector<double>& autocorrelation,
    std::vector<double>* linear_predictive_coefficients, bool* is_stable,
    LevinsonDurbinRecursion::Buffer* buffer) const {
  // Check inputs.
  const int length(num_order_ + 1);
  if (!is_valid_ ||
      autocorrelation.size() != static_cast<std::size_t>(length) ||
      NULL == linear_predictive_coefficients || NULL == is_stable ||
      NULL == buffer) {
    return false;
  }

  // Prepare memories.
  if (linear_predictive_coefficients->size() !=
      static_cast<std::size_t>(length)) {
    linear_predictive_coefficients->resize(length);
  }
  if (buffer->c_.size() != static_cast<std::size_t>(length)) {
    buffer->c_.resize(length);
  }

  *is_stable = true;

  const double* r(&(autocorrelation[0]));
  double* a(&((*linear_predictive_coefficients)[0]));
  double* c(&buffer->c_[0]);

  // Set initial condition.
  a[0] = 0.0;
  double e(r[0]);
  if (0.0 == e || std::isnan(e)) {
    return false;
  }

  // Perform Durbin's iterative algorithm.
  for (int i(1); i < length; ++i) {
    double k(-r[i]);
    for (int j(1); j < i; ++j) {
      k -= c[j] * r[i - j];
    }
    k /= e;

    if (1.0 <= std::fabs(k)) {
      *is_stable = false;
    }

    for (int j(1); j < i; ++j) {
      a[j] = c[j] + k * c[i - j];
    }
    a[i] = k;

    e *= 1.0 - k * k;
    if (0.0 == e || std::isnan(e)) {
      return false;
    }

    for (int j(0); j <= i; ++j) {
      c[j] = a[j];
    }
  }

  // Set gain.
  a[0] = std::sqrt(e);

  return true;
}

bool LevinsonDurbinRecursion::Run(
    std::vector<double>* input_and_output, bool* is_stable,
    LevinsonDurbinRecursion::Buffer* buffer) const {
  if (NULL == input_and_output) return false;
  std::vector<double> input(*input_and_output);
  return Run(input, input_and_output, is_stable, buffer);
}

}  // namespace sptk
