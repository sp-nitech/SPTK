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

#include "SPTK/math/frequency_transform.h"

#include <algorithm>  // std::copy, std::fill
#include <cstddef>    // std::size_t

namespace sptk {

FrequencyTransform::FrequencyTransform(int num_input_order,
                                       int num_output_order, double alpha)
    : num_input_order_(num_input_order),
      num_output_order_(num_output_order),
      alpha_(alpha),
      is_valid_(true) {
  if (num_input_order_ < 0 || num_output_order_ < 0 ||
      !sptk::IsValidAlpha(alpha_)) {
    is_valid_ = false;
    return;
  }
}

bool FrequencyTransform::Run(const std::vector<double>& minimum_phase_sequence,
                             std::vector<double>* warped_sequence,
                             FrequencyTransform::Buffer* buffer) const {
  // Check inputs.
  const int input_length(num_input_order_ + 1);
  if (!is_valid_ ||
      minimum_phase_sequence.size() != static_cast<std::size_t>(input_length) ||
      NULL == warped_sequence || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  const int output_length(num_output_order_ + 1);
  if (warped_sequence->size() != static_cast<std::size_t>(output_length)) {
    warped_sequence->resize(output_length);
  }
  if (buffer->d_.size() != static_cast<std::size_t>(output_length)) {
    buffer->d_.resize(output_length);
  }

  // There is no need to convert input when alpha is zero.
  if (0.0 == alpha_) {
    if (num_input_order_ < num_output_order_) {
      std::copy(minimum_phase_sequence.begin(), minimum_phase_sequence.end(),
                warped_sequence->begin());
      std::fill(warped_sequence->begin() + input_length, warped_sequence->end(),
                0.0);
    } else {
      std::copy(minimum_phase_sequence.begin(),
                minimum_phase_sequence.begin() + output_length,
                warped_sequence->begin());
    }
    return true;
  }

  std::fill(warped_sequence->begin(), warped_sequence->end(), 0.0);

  const double beta(1.0 - alpha_ * alpha_);
  const double* c(&(minimum_phase_sequence[0]));
  double* d(&buffer->d_[0]);
  double* g(&((*warped_sequence)[0]));

  // Apply recursive formula.
  for (int i(num_input_order_); 0 <= i; --i) {
    d[0] = g[0];
    g[0] = c[i] + alpha_ * d[0];
    if (1 <= num_output_order_) {
      d[1] = g[1];
      g[1] = beta * d[0] + alpha_ * d[1];
    }
    for (int m(2); m <= num_output_order_; ++m) {
      d[m] = g[m];
      g[m] = d[m - 1] + alpha_ * (d[m] - g[m - 1]);
    }
  }

  return true;
}

}  // namespace sptk
