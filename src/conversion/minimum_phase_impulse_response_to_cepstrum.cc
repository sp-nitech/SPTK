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

#include "SPTK/conversion/minimum_phase_impulse_response_to_cepstrum.h"

#include <cmath>    // std::log
#include <cstddef>  // std::size_t

namespace sptk {

MinimumPhaseImpulseResponseToCepstrum::MinimumPhaseImpulseResponseToCepstrum(
    int num_input_order, int num_output_order)
    : num_input_order_(num_input_order),
      num_output_order_(num_output_order),
      is_valid_(true) {
  if (num_input_order_ < 0 || num_output_order_ < 0) {
    is_valid_ = false;
    return;
  }
}

bool MinimumPhaseImpulseResponseToCepstrum::Run(
    const std::vector<double>& minimum_phase_impulse_response,
    std::vector<double>* cepstrum) const {
  // Check inputs.
  if (!is_valid_ ||
      minimum_phase_impulse_response.size() !=
          static_cast<std::size_t>(num_input_order_ + 1) ||
      NULL == cepstrum) {
    return false;
  }

  // Prepare memories.
  if (cepstrum->size() != static_cast<std::size_t>(num_output_order_ + 1)) {
    cepstrum->resize(num_output_order_ + 1);
  }

  const double* h(&minimum_phase_impulse_response[0]);
  double* c(&((*cepstrum)[0]));

  c[0] = std::log(h[0]);
  for (int n(1); n <= num_output_order_; ++n) {
    double sum(0.0);
    const int begin((num_input_order_ < n) ? n - num_input_order_ : 1);
    for (int k(begin); k < n; ++k) {
      sum -= k * c[k] * h[n - k];
    }
    if (n <= num_input_order_) {
      sum += n * h[n];
    }
    c[n] = sum / (n * h[0]);
  }

  return true;
}

}  // namespace sptk
