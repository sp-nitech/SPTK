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
