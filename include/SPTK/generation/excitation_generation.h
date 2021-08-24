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

#ifndef SPTK_GENERATION_EXCITATION_GENERATION_H_
#define SPTK_GENERATION_EXCITATION_GENERATION_H_

#include "SPTK/generation/random_generation_interface.h"
#include "SPTK/input/input_source_interpolation_with_magic_number.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Generate exication sequence.
 *
 * The input is a sequence of pitch value which can be either a continuous value
 * of a magic number. The output is the excitation signal given the input
 * sequence.
 */
class ExcitationGeneration {
 public:
  /**
   * @param[in] input_source Input source.
   * @param[in] random_generation Random value generator.
   */
  ExcitationGeneration(InputSourceInterpolationWithMagicNumber* input_source,
                       RandomGenerationInterface* random_generation);

  virtual ~ExcitationGeneration() {
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * Get excitation signal.
   *
   * @param[out] excitation Excitation (optional).
   * @param[out] pulse Pulse (optional).
   * @param[out] noise Noise (optional).
   * @param[out] pitch Pitch (optional).
   * @return True on success, false on failure.
   */
  bool Get(double* excitation, double* pulse, double* noise, double* pitch);

 private:
  InputSourceInterpolationWithMagicNumber* input_source_;
  RandomGenerationInterface* random_generation_;

  bool is_valid_;

  // Phase value ranging from 0.0 to 1.0.
  double phase_;

  DISALLOW_COPY_AND_ASSIGN(ExcitationGeneration);
};

}  // namespace sptk

#endif  // SPTK_GENERATION_EXCITATION_GENERATION_H_
