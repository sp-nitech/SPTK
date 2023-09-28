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

#ifndef SPTK_GENERATION_SINUSOIDAL_GENERATION_H_
#define SPTK_GENERATION_SINUSOIDAL_GENERATION_H_

#include "SPTK/input/input_source_interpolation_with_magic_number.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Generate sinusoidal sequence.
 *
 * The input is a sequence of pitch value which can be either a continuous value
 * or a magic number. The output is the sinusoidal signal given the input
 * sequence.
 */
class SinusoidalGeneration {
 public:
  /**
   * @param[in] input_source Input source.
   */
  explicit SinusoidalGeneration(
      InputSourceInterpolationWithMagicNumber* input_source);

  virtual ~SinusoidalGeneration() {
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * Get sinusoidal signal.
   *
   * @param[out] sin Sine waveform (optional).
   * @param[out] cos Cosine waveform (optional).
   * @param[out] pitch Pitch (optional).
   * @return True on success, false on failure.
   */
  bool Get(double* sin, double* cos, double* pitch);

 private:
  InputSourceInterpolationWithMagicNumber* input_source_;

  bool is_valid_;

  // Phase value ranging from 0.0 to 2 x pi
  double phase_;

  DISALLOW_COPY_AND_ASSIGN(SinusoidalGeneration);
};

}  // namespace sptk

#endif  // SPTK_GENERATION_SINUSOIDAL_GENERATION_H_
