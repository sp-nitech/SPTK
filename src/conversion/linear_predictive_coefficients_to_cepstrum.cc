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

#include "SPTK/conversion/linear_predictive_coefficients_to_cepstrum.h"

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
    return;
  }
}

bool LinearPredictiveCoefficientsToCepstrum::Run(
    const std::vector<double>& linear_predictive_coefficients,
    std::vector<double>* cepstrum) const {
  // Check inputs.
  if (!is_valid_ ||
      linear_predictive_coefficients.size() !=
          static_cast<std::size_t>(num_input_order_ + 1) ||
      NULL == cepstrum) {
    return false;
  }

  // Prepare memories.
  const int output_length(num_output_order_ + 1);
  if (cepstrum->size() != static_cast<std::size_t>(output_length)) {
    cepstrum->resize(output_length);
  }

  const double* input(&(linear_predictive_coefficients[0]));
  double* output(&((*cepstrum)[0]));

  output[0] = std::log(input[0]);
  if (0 == num_output_order_) return true;

  output[1] = -input[1];

  for (int m(2); m <= num_output_order_; ++m) {
    double sum(0.0);
    const int k_first((num_input_order_ < m) ? (m - num_input_order_) : 1);
    for (int k(k_first); k < m; ++k) {
      sum += k * output[k] * input[m - k];
    }
    output[m] = -sum / m;

    if (m <= num_input_order_) {
      output[m] -= input[m];
    }
  }

  return true;
}

}  // namespace sptk
