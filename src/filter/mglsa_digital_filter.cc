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

#include "SPTK/filter/mglsa_digital_filter.h"

#include <algorithm>  // std::fill
#include <cmath>      // std::exp
#include <cstddef>    // std::size_t

namespace sptk {

MglsaDigitalFilter::MglsaDigitalFilter(int num_filter_order, int num_pade_order,
                                       int num_stage, double alpha,
                                       bool transposition)
    : num_filter_order_(num_filter_order),
      num_stage_(num_stage),
      alpha_(alpha),
      transposition_(transposition),
      mlsa_digital_filter_(num_filter_order_, num_pade_order, alpha_,
                           transposition_),
      is_valid_(true) {
  if (num_filter_order_ < 0 || num_stage_ < 0 || !sptk::IsValidAlpha(alpha_)) {
    is_valid_ = false;
    return;
  }
  if (0 == num_stage && !mlsa_digital_filter_.IsValid()) {
    is_valid_ = false;
    return;
  }
}

bool MglsaDigitalFilter::Run(const std::vector<double>& filter_coefficients,
                             double filter_input, double* filter_output,
                             MglsaDigitalFilter::Buffer* buffer) const {
  // Check inputs.
  if (!is_valid_ ||
      filter_coefficients.size() !=
          static_cast<std::size_t>(num_filter_order_ + 1) ||
      NULL == filter_output || NULL == buffer) {
    return false;
  }

  // Use MLSA filter.
  if (0 == num_stage_) {
    return mlsa_digital_filter_.Run(filter_coefficients, filter_input,
                                    filter_output,
                                    &(buffer->mlsa_digital_filter_buffer_));
  }

  // Prepare memories.
  if (buffer->signals_.size() !=
      static_cast<std::size_t>((num_filter_order_ + 1) * num_stage_)) {
    buffer->signals_.resize((num_filter_order_ + 1) * num_stage_);
    std::fill(buffer->signals_.begin(), buffer->signals_.end(), 0.0);
  }

  const double gained_input(filter_input * std::exp(filter_coefficients[0]));
  if (0 == num_filter_order_) {
    *filter_output = gained_input;
    return true;
  }

  const double* b(&(filter_coefficients[1]));
  const double beta(1.0 - alpha_ * alpha_);
  double x(gained_input);

  for (int i(0); i < num_stage_; ++i) {
    double* d(&buffer->signals_[(num_filter_order_ + 1) * i]);
    if (transposition_) {
      x -= beta * d[0];
      d[num_filter_order_] =
          b[num_filter_order_ - 1] * x + alpha_ * d[num_filter_order_ - 1];
      for (int j(num_filter_order_ - 1); 0 < j; --j) {
        d[j] += b[j - 1] * x + alpha_ * (d[j - 1] - d[j + 1]);
      }

      for (int j(0); j < num_filter_order_; ++j) {
        d[j] = d[j + 1];
      }
    } else {
      double y(d[0] * b[0]);
      for (int j(1); j < num_filter_order_; ++j) {
        d[j] += alpha_ * (d[j + 1] - d[j - 1]);
        y += d[j] * b[j];
      }
      x -= y;

      for (int j(num_filter_order_); 0 < j; --j) {
        d[j] = d[j - 1];
      }
      d[0] = alpha_ * d[0] + beta * x;
    }
  }

  *filter_output = x;

  return true;
}

bool MglsaDigitalFilter::Run(const std::vector<double>& filter_coefficients,
                             double* input_and_output,
                             MglsaDigitalFilter::Buffer* buffer) const {
  if (NULL == input_and_output) return false;
  return Run(filter_coefficients, *input_and_output, input_and_output, buffer);
}

}  // namespace sptk
