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

#include "SPTK/compression/inverse_uniform_quantization.h"

#include <cmath>  // std::pow

namespace sptk {

InverseUniformQuantization::InverseUniformQuantization(
    double absolute_maximum_value, int num_bit,
    UniformQuantization::QuantizationType quantization_type)
    : absolute_maximum_value_(absolute_maximum_value),
      num_bit_(num_bit),
      quantization_type_(quantization_type),
      is_valid_(true) {
  if (absolute_maximum_value_ <= 0.0 || num_bit_ <= 0) {
    is_valid_ = false;
    return;
  }

  switch (quantization_type_) {
    case UniformQuantization::QuantizationType::kMidRise: {
      quantization_levels_ = static_cast<int>(std::pow(2.0, num_bit_));
      break;
    }
    case UniformQuantization::QuantizationType::kMidTread: {
      quantization_levels_ = static_cast<int>(std::pow(2.0, num_bit_)) - 1;
      break;
    }
    default: {
      is_valid_ = false;
      return;
    }
  }

  step_size_ = (2.0 * absolute_maximum_value_) / quantization_levels_;
}

bool InverseUniformQuantization::Run(int input, double* output) const {
  // Check inputs.
  if (!is_valid_ || NULL == output) {
    return false;
  }

  double value;
  switch (quantization_type_) {
    case UniformQuantization::QuantizationType::kMidRise: {
      value = (input - quantization_levels_ / 2 + 0.5) * step_size_;
      break;
    }
    case UniformQuantization::QuantizationType::kMidTread: {
      value = (input - (quantization_levels_ - 1) / 2) * step_size_;
      break;
    }
    default: {
      return false;
    }
  }

  // Clip value.
  if (value < -absolute_maximum_value_) {
    value = -absolute_maximum_value_;
  } else if (absolute_maximum_value_ < value) {
    value = absolute_maximum_value_;
  }

  *output = value;

  return true;
}

}  // namespace sptk
