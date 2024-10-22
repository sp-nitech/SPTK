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

#include "SPTK/filter/all_zero_lattice_digital_filter.h"

#include <algorithm>  // std::fill
#include <cstddef>    // std::size_t
#include <vector>     // std::vector

namespace sptk {

AllZeroLatticeDigitalFilter::AllZeroLatticeDigitalFilter(int num_filter_order)
    : num_filter_order_(num_filter_order), is_valid_(true) {
  if (num_filter_order_ < 0) {
    is_valid_ = false;
    return;
  }
}

bool AllZeroLatticeDigitalFilter::Run(
    const std::vector<double>& filter_coefficients, double filter_input,
    double* filter_output, AllZeroLatticeDigitalFilter::Buffer* buffer) const {
  // Check inputs
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

  if (0 == num_filter_order_) {
    *filter_output = filter_input * filter_coefficients[0];
    return true;
  }

  const double* k(&(filter_coefficients[1]));
  double* d(&buffer->d_[0]);
  double sum(filter_input);
  double next_d(filter_input);

  // Apply all-zero lattice filter.
  for (int m(0); m < num_filter_order_; ++m) {
    const double tmp(d[m] + k[m] * sum);
    sum += k[m] * d[m];
    d[m] = next_d;
    next_d = tmp;
  }

  // Save result.
  *filter_output = sum * filter_coefficients[0];

  return true;
}

bool AllZeroLatticeDigitalFilter::Run(
    const std::vector<double>& filter_coefficients, double* input_and_output,
    AllZeroLatticeDigitalFilter::Buffer* buffer) const {
  if (NULL == input_and_output) return false;
  return Run(filter_coefficients, *input_and_output, input_and_output, buffer);
}

}  // namespace sptk
