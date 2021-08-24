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

#include "SPTK/conversion/composite_sinusoidal_modeling_to_autocorrelation.h"

#include <cmath>    // std::cos
#include <cstddef>  // std::size_t

namespace sptk {

CompositeSinusoidalModelingToAutocorrelation::
    CompositeSinusoidalModelingToAutocorrelation(int num_sine_wave)
    : num_sine_wave_(num_sine_wave), is_valid_(true) {
  if (num_sine_wave_ <= 0) {
    is_valid_ = false;
    return;
  }
}

bool CompositeSinusoidalModelingToAutocorrelation::Run(
    const std::vector<double>& composite_sinusoidal_modeling,
    std::vector<double>* autocorrelation) const {
  // Check inputs.
  const int length(num_sine_wave_ * 2);
  if (!is_valid_ ||
      composite_sinusoidal_modeling.size() !=
          static_cast<std::size_t>(length) ||
      NULL == autocorrelation) {
    return false;
  }

  // Prepare memories.
  if (autocorrelation->size() != static_cast<std::size_t>(length)) {
    autocorrelation->resize(length);
  }

  const double* frequencies(&(composite_sinusoidal_modeling[0]));
  const double* intensities(&(composite_sinusoidal_modeling[num_sine_wave_]));
  double* v(&((*autocorrelation)[0]));

  // Calculate autocorrelation using Eq. (2).
  for (int l(0); l < length; ++l) {
    double sum(0.0);
    for (int i(0); i < num_sine_wave_; ++i) {
      sum += intensities[i] * std::cos(l * frequencies[i]);
    }
    v[l] = sum;
  }

  return true;
}

bool CompositeSinusoidalModelingToAutocorrelation::Run(
    std::vector<double>* input_and_output) const {
  if (NULL == input_and_output) return false;
  std::vector<double> input(*input_and_output);
  return Run(input, input_and_output);
}

}  // namespace sptk
