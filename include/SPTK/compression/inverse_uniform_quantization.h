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

#ifndef SPTK_COMPRESSION_INVERSE_UNIFORM_QUANTIZATION_H_
#define SPTK_COMPRESSION_INVERSE_UNIFORM_QUANTIZATION_H_

#include "SPTK/compression/uniform_quantization.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Perform inverse uniform quantization.
 *
 * The input is an index and the output is the corresponding value.
 * There are two types of quantization, i.e., mid-rise and mid-tread.
 */
class InverseUniformQuantization {
 public:
  /**
   * @param[in] absolute_maximum_value Absolute maximum value.
   * @param[in] num_bit Number of quantization bits.
   * @param[in] quantization_type Quantization type.
   */
  InverseUniformQuantization(
      double absolute_maximum_value, int num_bit,
      UniformQuantization::QuantizationType quantization_type);

  virtual ~InverseUniformQuantization() {
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
  UniformQuantization::QuantizationType GetQuantizationType() const {
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
   * @param[in] input Input index.
   * @param[out] output Output value.
   * @return True on success, false on failure.
   */
  bool Run(int input, double* output) const;

 private:
  const double absolute_maximum_value_;
  const int num_bit_;
  const UniformQuantization::QuantizationType quantization_type_;

  bool is_valid_;

  int quantization_levels_;
  double step_size_;

  DISALLOW_COPY_AND_ASSIGN(InverseUniformQuantization);
};

}  // namespace sptk

#endif  // SPTK_COMPRESSION_INVERSE_UNIFORM_QUANTIZATION_H_
