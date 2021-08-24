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

#include "SPTK/conversion/cepstrum_to_minimum_phase_impulse_response.h"

#include <cmath>    // std::exp
#include <cstddef>  // std::size_t

namespace sptk {

CepstrumToMinimumPhaseImpulseResponse::CepstrumToMinimumPhaseImpulseResponse(
    int num_input_order, int num_output_order)
    : num_input_order_(num_input_order),
      num_output_order_(num_output_order),
      is_valid_(true) {
  if (num_input_order_ < 0 || num_output_order_ < 0) {
    is_valid_ = false;
    return;
  }
}

bool CepstrumToMinimumPhaseImpulseResponse::Run(
    const std::vector<double>& cepstrum,
    std::vector<double>* minimum_phase_impulse_response) const {
  // Check inputs.
  if (!is_valid_ ||
      cepstrum.size() != static_cast<std::size_t>(num_input_order_ + 1) ||
      NULL == minimum_phase_impulse_response) {
    return false;
  }

  // Prepare memories.
  if (minimum_phase_impulse_response->size() !=
      static_cast<std::size_t>(num_output_order_ + 1)) {
    minimum_phase_impulse_response->resize(num_output_order_ + 1);
  }

  const double* c(&cepstrum[0]);
  double* h(&((*minimum_phase_impulse_response)[0]));

  h[0] = std::exp(c[0]);
  for (int n(1); n <= num_output_order_; ++n) {
    double sum(0.0);
    const int end((num_input_order_ < n) ? num_input_order_ : n);
    for (int k(1); k <= end; ++k) {
      sum += k * c[k] * h[n - k];
    }
    h[n] = sum / n;
  }

  return true;
}

}  // namespace sptk
