// ----------------------------------------------------------------- //
//             The Speech Signal Processing Toolkit (SPTK)           //
//             developed by SPTK Working Group                       //
//             http://sp-tk.sourceforge.net/                         //
// ----------------------------------------------------------------- //
//                                                                   //
//  Copyright (c) 1984-2007  Tokyo Institute of Technology           //
//                           Interdisciplinary Graduate School of    //
//                           Science and Engineering                 //
//                                                                   //
//                1996-2020  Nagoya Institute of Technology          //
//                           Department of Computer Science          //
//                                                                   //
// All rights reserved.                                              //
//                                                                   //
// Redistribution and use in source and binary forms, with or        //
// without modification, are permitted provided that the following   //
// conditions are met:                                               //
//                                                                   //
// - Redistributions of source code must retain the above copyright  //
//   notice, this list of conditions and the following disclaimer.   //
// - Redistributions in binary form must reproduce the above         //
//   copyright notice, this list of conditions and the following     //
//   disclaimer in the documentation and/or other materials provided //
//   with the distribution.                                          //
// - Neither the name of the SPTK working group nor the names of its //
//   contributors may be used to endorse or promote products derived //
//   from this software without specific prior written permission.   //
//                                                                   //
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            //
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       //
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          //
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          //
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS //
// BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          //
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   //
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     //
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON //
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   //
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    //
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           //
// POSSIBILITY OF SUCH DAMAGE.                                       //
// ----------------------------------------------------------------- //

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
