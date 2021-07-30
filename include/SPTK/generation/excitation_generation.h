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
//                1996-2021  Nagoya Institute of Technology          //
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
