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

#include "SPTK/generation/sinusoidal_generation.h"

#include <cmath>   // std::cos, std::sin
#include <vector>  // std::vector

namespace sptk {

SinusoidalGeneration::SinusoidalGeneration(
    InputSourceInterpolationWithMagicNumber* input_source)
    : input_source_(input_source), is_valid_(true), phase_(0.0) {
  if (NULL == input_source_ || !input_source_->IsValid()) {
    is_valid_ = false;
    return;
  }
}

bool SinusoidalGeneration::Get(double* sin, double* cos, double* pitch) {
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

  if (pitch) {
    *pitch = pitch_in_current_point;
  }

  // If unvoiced point, return zero.
  if (input_source_->GetMagicNumber() == pitch_in_current_point) {
    phase_ = 0.0;
    if (sin) {
      *sin = 0.0;
    }
    if (cos) {
      *cos = 0.0;
    }
    return true;
  }

  if (sin) {
    *sin = std::sin(phase_);
  }
  if (cos) {
    *cos = std::cos(phase_);
  }

  // Proceed phase.
  phase_ += sptk::kTwoPi / pitch_in_current_point;
  if (sptk::kTwoPi < phase_) {
    phase_ -= sptk::kTwoPi;
  }

  return true;
}

}  // namespace sptk
