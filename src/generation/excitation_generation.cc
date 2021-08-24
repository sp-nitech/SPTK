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

#include "SPTK/generation/excitation_generation.h"

#include <cmath>   // std::sqrt
#include <vector>  // std::vector

namespace sptk {

ExcitationGeneration::ExcitationGeneration(
    InputSourceInterpolationWithMagicNumber* input_source,
    RandomGenerationInterface* random_generation)
    : input_source_(input_source),
      random_generation_(random_generation),
      is_valid_(true),
      phase_(1.0) {
  if (NULL == input_source_ || NULL == random_generation_ ||
      !input_source_->IsValid()) {
    is_valid_ = false;
    return;
  }
}

bool ExcitationGeneration::Get(double* excitation, double* pulse, double* noise,
                               double* pitch) {
  if (!is_valid_) {
    return false;
  }

  // Get pitch.
  double pitch_in_current_point;
  {
    std::vector<double> tmp;
    if (!input_source_->Get(&tmp) || tmp[0] < 0.0) {
      return false;
    }
    pitch_in_current_point = tmp[0];
  }

  // Get noise.
  double noise_in_current_point;
  if (!random_generation_->Get(&noise_in_current_point)) {
    return false;
  }

  if (pitch) {
    *pitch = pitch_in_current_point;
  }
  if (noise) {
    *noise = noise_in_current_point;
  }

  // If unvoiced point, return white noise.
  if (input_source_->GetMagicNumber() == pitch_in_current_point) {
    phase_ = 1.0;
    if (excitation) {
      *excitation = noise_in_current_point;
    }
    if (pulse) {
      *pulse = 0.0;
    }
    return true;
  }

  // If voiced point, return pulse or zero.
  double pulse_in_current_point;
  if (1.0 <= phase_) {
    phase_ -= 1.0;
    pulse_in_current_point = std::sqrt(pitch_in_current_point);
  } else {
    pulse_in_current_point = 0.0;
  }

  if (excitation) {
    *excitation = pulse_in_current_point;
  }
  if (pulse) {
    *pulse = pulse_in_current_point;
  }

  // Proceed phase.
  phase_ += 1.0 / pitch_in_current_point;

  return true;
}

}  // namespace sptk
