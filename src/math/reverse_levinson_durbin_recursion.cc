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

#include "SPTK/math/reverse_levinson_durbin_recursion.h"

#include <cstddef>  // std::size_t

namespace sptk {

ReverseLevinsonDurbinRecursion::ReverseLevinsonDurbinRecursion(int num_order)
    : num_order_(num_order), is_valid_(true) {
  if (num_order_ < 0) {
    is_valid_ = false;
    return;
  }
}

bool ReverseLevinsonDurbinRecursion::Run(
    const std::vector<double>& linear_predictive_coefficients,
    std::vector<double>* autocorrelation,
    ReverseLevinsonDurbinRecursion::Buffer* buffer) const {
  // Check inputs.
  const int length(num_order_ + 1);
  if (!is_valid_ ||
      linear_predictive_coefficients.size() !=
          static_cast<std::size_t>(length) ||
      NULL == autocorrelation || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  if (autocorrelation->size() != static_cast<std::size_t>(length)) {
    autocorrelation->resize(length);
  }
  if (buffer->u_.GetNumDimension() != length) {
    buffer->u_.Resize(length);
  }
  if (buffer->e_.size() != static_cast<std::size_t>(length)) {
    buffer->e_.resize(length);
  }

  const double* a(&(linear_predictive_coefficients[0]));
  double* r(&((*autocorrelation)[0]));
  double* e(&(buffer->e_[0]));

  for (int j(0); j <= num_order_; ++j) {
    buffer->u_[j][j] = 1.0;
  }
  for (int j(0); j < num_order_; ++j) {
    buffer->u_[num_order_][j] = a[num_order_ - j];
  }
  e[num_order_] = a[0] * a[0];

  for (int i(num_order_ - 1); 0 <= i; --i) {
    double* u(&(buffer->u_[i + 1][0]));
    const double t(1.0 / (1.0 - u[0] * u[0]));
    for (int j(0); j < i; ++j) {
      buffer->u_[i][j] = (u[j + 1] - u[0] * u[i - j]) * t;
    }
    e[i] = e[i + 1] * t;
  }

  r[0] = e[0];
  for (int i(1); i <= num_order_; ++i) {
    double sum(0.0);
    for (int j(1); j < i; ++j) {
      sum += buffer->u_[i - 1][i - j - 1] * r[i - j];
    }
    r[i] = -(sum + buffer->u_[i][0] * e[i - 1]);
  }

  return true;
}

bool ReverseLevinsonDurbinRecursion::Run(
    std::vector<double>* input_and_output,
    ReverseLevinsonDurbinRecursion::Buffer* buffer) const {
  if (NULL == input_and_output) return false;
  return Run(*input_and_output, input_and_output, buffer);
}

}  // namespace sptk
