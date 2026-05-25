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

#include "SPTK/math/lp_norm_calculation.h"

#include <algorithm>  // std::max_element
#include <cmath>      // std::abs, std::pow
#include <cstddef>    // std::size_t
#include <limits>     // std::numeric_limits
#include <numeric>    // std::inner_product
#include <vector>     // std::vector

namespace sptk {

LpNormCalculation::LpNormCalculation(int num_order, double p)
    : num_order_(num_order), p_(p), is_valid_(true) {
  if (num_order_ < 0 || p_ < 1.0) {
    is_valid_ = false;
    return;
  }
}

bool LpNormCalculation::Run(const std::vector<double>& data,
                            double* norm) const {
  if (!is_valid_ || data.size() != static_cast<std::size_t>(num_order_ + 1) ||
      NULL == norm) {
    return false;
  }

  // Specialize for p = 2 as it is the most common case and can be computed
  // faster than the general case.
  if (p_ == 2.0) {
    *norm = std::sqrt(
        std::inner_product(data.begin(), data.end(), data.begin(), 0.0));
    return true;
  }

  const double max_value(std::abs(*std::max_element(
      data.begin(), data.end(),
      [](double a, double b) { return std::abs(a) < std::abs(b); })));

  if (0.0 == max_value) {
    *norm = 0.0;
    return true;
  }

  if (p_ == std::numeric_limits<double>::infinity()) {
    *norm = max_value;
    return true;
  }

  const double* input(&(data[0]));
  double sum(0.0);
  for (int i(0); i <= num_order_; ++i) {
    sum += std::pow(std::abs(input[i]) / max_value, p_);
  }
  *norm = max_value * std::pow(sum, 1.0 / p_);

  return true;
}

}  // namespace sptk
