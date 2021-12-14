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

#include "SPTK/compression/uniform_quantization.h"

#include <cmath>  // std::floor, std::pow, std::round

namespace sptk {

UniformQuantization::UniformQuantization(double absolute_maximum_value,
                                         int num_bit,
                                         QuantizationType quantization_type)
    : absolute_maximum_value_(absolute_maximum_value),
      num_bit_(num_bit),
      quantization_type_(quantization_type),
      is_valid_(true) {
  if (absolute_maximum_value_ <= 0.0 || num_bit_ <= 0) {
    is_valid_ = false;
    return;
  }

  switch (quantization_type_) {
    case kMidRise: {
      quantization_levels_ = static_cast<int>(std::pow(2.0, num_bit_));
      break;
    }
    case kMidTread: {
      quantization_levels_ = static_cast<int>(std::pow(2.0, num_bit_)) - 1;
      break;
    }
    default: {
      is_valid_ = false;
      return;
    }
  }

  inverse_step_size_ = quantization_levels_ / (2.0 * absolute_maximum_value_);
}

bool UniformQuantization::Run(double input, int* output) const {
  // Check inputs.
  if (!is_valid_ || NULL == output) {
    return false;
  }

  int index;
  switch (quantization_type_) {
    case kMidRise: {
      index = static_cast<int>(std::floor(input * inverse_step_size_) +
                               quantization_levels_ / 2);
      break;
    }
    case kMidTread: {
      index = static_cast<int>(std::round(input * inverse_step_size_) +
                               (quantization_levels_ - 1) / 2);
      break;
    }
    default: {
      return false;
    }
  }

  // Clip index.
  if (index < 0) {
    index = 0;
  } else if (quantization_levels_ <= index) {
    index = quantization_levels_ - 1;
  }

  *output = index;

  return true;
}

}  // namespace sptk
