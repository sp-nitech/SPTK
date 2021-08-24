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

#include "SPTK/filter/infinite_impulse_response_digital_filter.h"

#include <algorithm>  // std::fill, std::max
#include <cstddef>    // std::size_t

namespace {

void MovePointer(int max, int* p) {
  --(*p);
  if (*p < 0) *p = max;
}

}  // namespace

namespace sptk {

InfiniteImpulseResponseDigitalFilter::InfiniteImpulseResponseDigitalFilter(
    const std::vector<double>& denominator_coefficients,
    const std::vector<double>& numerator_coefficients)
    : denominator_coefficients_(denominator_coefficients),
      numerator_coefficients_(numerator_coefficients),
      num_denominator_order_(static_cast<int>(denominator_coefficients.size()) -
                             1),
      num_numerator_order_(static_cast<int>(numerator_coefficients.size()) - 1),
      num_filter_order_(std::max(num_denominator_order_, num_numerator_order_)),
      is_valid_(true) {
  if (num_denominator_order_ < 0 || num_numerator_order_ < 0) {
    is_valid_ = false;
    return;
  }
}

bool InfiniteImpulseResponseDigitalFilter::Run(
    double input, double* output,
    InfiniteImpulseResponseDigitalFilter::Buffer* buffer) const {
  // Check inputs.
  if (!is_valid_ || NULL == output || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  if (buffer->d_.size() != static_cast<std::size_t>(num_filter_order_ + 1)) {
    buffer->d_.resize(num_filter_order_ + 1);
    std::fill(buffer->d_.begin(), buffer->d_.end(), 0.0);
    buffer->p_ = 0;
  }

  double* d(&buffer->d_[0]);

  {
    const double* a(&(denominator_coefficients_[0]));
    double x(-input * a[0]);
    for (int i(1), p(buffer->p_); i <= num_denominator_order_; ++i) {
      MovePointer(num_filter_order_, &p);
      x += d[p] * a[i];
    }
    d[buffer->p_] = -x;
  }

  {
    const double* b(&(numerator_coefficients_[0]));
    double y(0.0);
    for (int i(0), p(buffer->p_ + 1); i <= num_numerator_order_; ++i) {
      MovePointer(num_filter_order_, &p);
      y += d[p] * b[i];
    }
    *output = y;
  }

  // Update pointer of ring buffer.
  ++(buffer->p_);
  if (num_filter_order_ < buffer->p_) {
    buffer->p_ = 0;
  }

  return true;
}

bool InfiniteImpulseResponseDigitalFilter::Run(
    double* input_and_output,
    InfiniteImpulseResponseDigitalFilter::Buffer* buffer) const {
  if (NULL == input_and_output) return false;
  return Run(*input_and_output, input_and_output, buffer);
}

}  // namespace sptk
