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

#include "SPTK/math/vandermonde_system_solver.h"

#include <algorithm>  // std::fill
#include <cstddef>    // std::size_t

namespace sptk {

VandermondeSystemSolver::VandermondeSystemSolver(int num_order)
    : num_order_(num_order), is_valid_(true) {
  if (num_order_ < 0) {
    is_valid_ = false;
    return;
  }
}

bool VandermondeSystemSolver::Run(
    const std::vector<double>& coefficient_vector,
    const std::vector<double>& constant_vector,
    std::vector<double>* solution_vector,
    VandermondeSystemSolver::Buffer* buffer) const {
  // Check inputs.
  const int length(num_order_ + 1);
  if (!is_valid_ ||
      coefficient_vector.size() != static_cast<std::size_t>(length) ||
      constant_vector.size() != static_cast<std::size_t>(length) ||
      NULL == solution_vector || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  if (solution_vector->size() != static_cast<std::size_t>(length)) {
    solution_vector->resize(length);
  }
  if (buffer->d_.size() != static_cast<std::size_t>(length)) {
    buffer->d_.resize(length);
  }

  const double* a(&(coefficient_vector[0]));
  const double* b(&(constant_vector[0]));
  double* x(&((*solution_vector)[0]));
  double* d(&(buffer->d_[0]));

  std::fill(buffer->d_.begin(), buffer->d_.end(), 0.0);
  for (int i(0); i < length; ++i) {
    for (int j(i); 0 < j; --j) {
      d[j] -= a[i] * d[j - 1];
    }
    d[0] -= a[i];
  }

  for (int i(0); i <= num_order_; ++i) {
    double tmp(1.0);
    double numerator(b[num_order_]);
    double denominator(1.0);
    for (int j(0); j < num_order_; ++j) {
      tmp = d[j] + a[i] * tmp;
      numerator = numerator + b[num_order_ - j - 1] * tmp;
      denominator = denominator * a[i] + tmp;
    }
    if (0.0 == denominator) {
      return false;
    }
    x[i] = numerator / denominator;
  }

  return true;
}

}  // namespace sptk
