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

#include "SPTK/generation/periodic_waveform_generation.h"

#include <cmath>   // std::cos, std::sin
#include <vector>  // std::vector

namespace sptk {

PeriodicWaveformGeneration::PeriodicWaveformGeneration(
    double unvoiced_value, bool strict,
    InputSourceInterpolationWithMagicNumber* input_source)
    : unvoiced_value_(unvoiced_value),
      strict_(strict),
      input_source_(input_source),
      is_valid_(true),
      phase_(0.0),
      voiced_pitch_(0.0),
      extending_(false) {
  if (NULL == input_source_ || !input_source_->IsValid()) {
    is_valid_ = false;
    return;
  }
}

bool PeriodicWaveformGeneration::Get(double* sin, double* cos, double* sawtooth,
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

  const bool unvoiced(input_source_->GetMagicNumber() ==
                      pitch_in_current_point);
  if (!strict_ && unvoiced && extending_) {
    pitch_in_current_point = voiced_pitch_;
  }

  if (pitch) {
    *pitch = pitch_in_current_point;
  }

  // If unvoiced point, return zero.
  if (input_source_->GetMagicNumber() == pitch_in_current_point) {
    phase_ = 0.0;
    if (sin) {
      *sin = unvoiced_value_;
    }
    if (cos) {
      *cos = unvoiced_value_;
    }
    if (sawtooth) {
      *sawtooth = unvoiced_value_;
    }
    return true;
  }

  if (sin) {
    *sin = std::sin(phase_);
  }
  if (cos) {
    *cos = std::cos(phase_);
  }
  if (sawtooth) {
    *sawtooth = phase_ / sptk::kPi - 1.0;
  }

  if (!strict_) {
    extending_ = true;
    voiced_pitch_ = pitch_in_current_point;
  }

  // Proceed phase.
  const double prev_phase(phase_);
  phase_ += sptk::kTwoPi / pitch_in_current_point;
  if (!strict_ && unvoiced &&
      ((prev_phase <= sptk::kPi && sptk::kPi <= phase_) ||
       (prev_phase <= sptk::kTwoPi && sptk::kTwoPi <= phase_))) {
    extending_ = false;
  }
  if (sptk::kTwoPi < phase_) {
    phase_ -= sptk::kTwoPi;
  }

  return true;
}

}  // namespace sptk
