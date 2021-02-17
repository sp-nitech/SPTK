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

#ifndef SPTK_COMPRESSION_UNIFORM_QUANTIZATION_H_
#define SPTK_COMPRESSION_UNIFORM_QUANTIZATION_H_

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Perform uniform quantization.
 *
 * The input is a value and the output is the corresponding index.
 * There are two types of quantization, i.e., mid-rise and mid-tread.
 */
class UniformQuantization {
 public:
  /**
   * Quantization type.
   */
  enum QuantizationType { kMidRise = 0, kMidTread, kNumQuantizationTypes };

  /**
   * @param[in] absolute_maximum_value Absolute maximum value.
   * @param[in] num_bit Number of quantization bits.
   * @param[in] quantization_type Quantization type.
   */
  UniformQuantization(double absolute_maximum_value, int num_bit,
                      QuantizationType quantization_type);

  virtual ~UniformQuantization() {
  }

  /**
   * @return Absolute maximum value.
   */
  double GetAbsoluteMaximumValue() const {
    return absolute_maximum_value_;
  }

  /**
   * @return Number of quantization bits.
   */
  int GetNumBit() const {
    return num_bit_;
  }

  /**
   * @return Quantization type.
   */
  QuantizationType GetQuantizationType() const {
    return quantization_type_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @return Quantization levels.
   */
  int GetQuantizationLevels() const {
    return quantization_levels_;
  }

  /**
   * @param[in] input Input value.
   * @param[out] output Output index.
   * @return True on success, false on failure.
   */
  bool Run(double input, int* output) const;

 private:
  const double absolute_maximum_value_;
  const int num_bit_;
  const QuantizationType quantization_type_;

  bool is_valid_;

  int quantization_levels_;
  double inverse_step_size_;

  DISALLOW_COPY_AND_ASSIGN(UniformQuantization);
};

}  // namespace sptk

#endif  // SPTK_COMPRESSION_UNIFORM_QUANTIZATION_H_
