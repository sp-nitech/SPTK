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

#include "SPTK/filter/all_pole_digital_filter.h"

#include <algorithm>  // std::fill
#include <cstddef>    // std::size_t

namespace sptk {

AllPoleDigitalFilter::AllPoleDigitalFilter(int num_filter_order,
                                           bool transposition)
    : num_filter_order_(num_filter_order),
      transposition_(transposition),
      is_valid_(true) {
  if (num_filter_order_ < 0) {
    is_valid_ = false;
    return;
  }
}

bool AllPoleDigitalFilter::Run(const std::vector<double>& filter_coefficients,
                               double filter_input, double* filter_output,
                               AllPoleDigitalFilter::Buffer* buffer) const {
  // Check inputs.
  if (!is_valid_ ||
      filter_coefficients.size() !=
          static_cast<std::size_t>(num_filter_order_ + 1) ||
      NULL == filter_output || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  if (buffer->d_.size() != static_cast<std::size_t>(num_filter_order_)) {
    buffer->d_.resize(num_filter_order_);
    std::fill(buffer->d_.begin(), buffer->d_.end(), 0.0);
  }

  const double gained_input(filter_input * filter_coefficients[0]);
  if (0 == num_filter_order_) {
    *filter_output = gained_input;
    return true;
  }

  const double* a(&(filter_coefficients[1]));
  double* d(&buffer->d_[0]);
  double sum(gained_input);

  // Apply all-pole filter.
  if (transposition_) {
    sum -= d[0];
    for (int m(1); m < num_filter_order_; ++m) {
      d[m - 1] = d[m] + a[m - 1] * sum;
    }
    d[num_filter_order_ - 1] = a[num_filter_order_ - 1] * sum;
  } else {
    for (int m(num_filter_order_ - 1); 0 < m; --m) {
      sum -= a[m] * d[m];
      d[m] = d[m - 1];
    }
    sum -= a[0] * d[0];
    d[0] = sum;
  }

  // Save result.
  *filter_output = sum;

  return true;
}

bool AllPoleDigitalFilter::Run(const std::vector<double>& filter_coefficients,
                               double* input_and_output,
                               AllPoleDigitalFilter::Buffer* buffer) const {
  if (NULL == input_and_output) return false;
  return Run(filter_coefficients, *input_and_output, input_and_output, buffer);
}

}  // namespace sptk
