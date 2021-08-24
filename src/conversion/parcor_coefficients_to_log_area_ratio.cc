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

#include "SPTK/conversion/parcor_coefficients_to_log_area_ratio.h"

#include <algorithm>  // std::transform
#include <cmath>      // std::atanh
#include <cstddef>    // std::size_t

namespace sptk {

ParcorCoefficientsToLogAreaRatio::ParcorCoefficientsToLogAreaRatio(
    int num_order)
    : num_order_(num_order), is_valid_(true) {
  if (num_order_ < 0) {
    is_valid_ = false;
    return;
  }
}

bool ParcorCoefficientsToLogAreaRatio::Run(
    const std::vector<double>& parcor_coefficients,
    std::vector<double>* log_area_ratio) const {
  // Check inputs.
  const int length(num_order_ + 1);
  if (!is_valid_ ||
      parcor_coefficients.size() != static_cast<std::size_t>(length) ||
      NULL == log_area_ratio) {
    return false;
  }

  // Prepare memories.
  if (log_area_ratio->size() != static_cast<std::size_t>(length)) {
    log_area_ratio->resize(length);
  }

  // Copy gain.
  (*log_area_ratio)[0] = parcor_coefficients[0];
  if (0 == num_order_) {
    return true;
  }

  std::transform(parcor_coefficients.begin() + 1, parcor_coefficients.end(),
                 log_area_ratio->begin() + 1,
                 [](double k) { return 2.0 * std::atanh(k); });

  return true;
}

bool ParcorCoefficientsToLogAreaRatio::Run(
    std::vector<double>* input_and_output) const {
  if (NULL == input_and_output) return false;
  return Run(*input_and_output, input_and_output);
}

}  // namespace sptk
