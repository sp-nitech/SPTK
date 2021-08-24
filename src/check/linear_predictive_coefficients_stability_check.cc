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

#include "SPTK/check/linear_predictive_coefficients_stability_check.h"

#include <cmath>    // std::fabs
#include <cstddef>  // std::size_t

namespace {

const double kMinimumMargin(1e-16);

}  // namespace

namespace sptk {

LinearPredictiveCoefficientsStabilityCheck::
    LinearPredictiveCoefficientsStabilityCheck(int num_order, double margin)
    : num_order_(num_order),
      margin_(margin),
      linear_predictive_coefficients_to_parcor_coefficients_(num_order_, 1.0),
      parcor_coefficients_to_linear_predictive_coefficients_(num_order_),
      is_valid_(true) {
  if (num_order_ < 0 || margin_ < kMinimumMargin || 1.0 <= margin_ ||
      !linear_predictive_coefficients_to_parcor_coefficients_.IsValid() ||
      !parcor_coefficients_to_linear_predictive_coefficients_.IsValid()) {
    is_valid_ = false;
    return;
  }
}

bool LinearPredictiveCoefficientsStabilityCheck::Run(
    const std::vector<double>& linear_predictive_coefficients,
    std::vector<double>* modified_linear_predictive_coefficients,
    bool* is_stable,
    LinearPredictiveCoefficientsStabilityCheck::Buffer* buffer) const {
  // Check inputs.
  if (!is_valid_ ||
      linear_predictive_coefficients.size() !=
          static_cast<std::size_t>(num_order_ + 1) ||
      NULL == is_stable || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  if (NULL != modified_linear_predictive_coefficients &&
      modified_linear_predictive_coefficients->size() !=
          static_cast<std::size_t>(num_order_ + 1)) {
    modified_linear_predictive_coefficients->resize(num_order_ + 1);
  }

  // Handle a special case.
  *is_stable = true;
  if (0 == num_order_) {
    if (NULL != modified_linear_predictive_coefficients) {
      (*modified_linear_predictive_coefficients)[0] =
          linear_predictive_coefficients[0];
    }
    return true;
  }

  // Check stability.
  if (!linear_predictive_coefficients_to_parcor_coefficients_.Run(
          linear_predictive_coefficients, &buffer->parcor_coefficients_,
          is_stable, &buffer->conversion_buffer_)) {
    return false;
  }

  // Perform modification.
  if (NULL != modified_linear_predictive_coefficients) {
    double* parcor_coefficients(&buffer->parcor_coefficients_[0]);
    const double bound(1.0 - margin_);
    for (int m(1); m <= num_order_; ++m) {
      if (bound < std::fabs(parcor_coefficients[m])) {
        parcor_coefficients[m] =
            (0.0 < parcor_coefficients[m]) ? bound : -bound;
      }
    }
    if (!parcor_coefficients_to_linear_predictive_coefficients_.Run(
            buffer->parcor_coefficients_,
            modified_linear_predictive_coefficients,
            &buffer->reconversion_buffer_)) {
      return false;
    }
  }

  return true;
}

bool LinearPredictiveCoefficientsStabilityCheck::Run(
    std::vector<double>* input_and_output, bool* is_stable,
    LinearPredictiveCoefficientsStabilityCheck::Buffer* buffer) const {
  if (NULL == input_and_output) return false;
  return Run(*input_and_output, input_and_output, is_stable, buffer);
}

}  // namespace sptk
