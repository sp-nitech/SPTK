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

#include "SPTK/window/data_windowing.h"

#include <algorithm>  // std::fill, std::transform
#include <cmath>      // std::sqrt
#include <cstddef>    // std::size_t
#include <numeric>    // std::accumulate, std::inner_product

namespace sptk {

DataWindowing::DataWindowing(WindowInterface* window_interface,
                             int output_length,
                             NormalizationType normalization_type)
    : input_length_(window_interface ? window_interface->GetWindowLength() : 0),
      output_length_(output_length),
      is_valid_(true) {
  if (input_length_ <= 0 || output_length_ < input_length_) {
    is_valid_ = false;
    return;
  }

  // Get window.
  window_ = window_interface->Get();

  double normalization_constant(1.0);
  switch (normalization_type) {
    case kNone: {
      // nothing to do
      break;
    }
    case kPower: {
      const double power(std::inner_product(window_.begin(), window_.end(),
                                            window_.begin(), 0.0));
      normalization_constant = 1.0 / std::sqrt(power);
      break;
    }
    case kMagnitude: {
      const double magnitude(
          std::accumulate(window_.begin(), window_.end(), 0.0));
      normalization_constant = 1.0 / magnitude;
      break;
    }
    default: {
      is_valid_ = false;
      return;
    }
  }

  if (1.0 != normalization_constant) {
    std::transform(window_.begin(), window_.end(), window_.begin(),
                   [normalization_constant](double w) {
                     return w * normalization_constant;
                   });
  }
}

bool DataWindowing::Run(const std::vector<double>& data,
                        std::vector<double>* windowed_data) const {
  // Check inputs.
  if (!is_valid_ || data.size() != static_cast<std::size_t>(input_length_) ||
      NULL == windowed_data) {
    return false;
  }

  // Prepare memories.
  if (windowed_data->size() != static_cast<std::size_t>(output_length_)) {
    windowed_data->resize(output_length_);
  }

  // Apply window.
  std::transform(data.begin(), data.begin() + input_length_, window_.begin(),
                 windowed_data->begin(),
                 [](double x, double w) { return x * w; });

  // Fill zero.
  std::fill(windowed_data->begin() + input_length_, windowed_data->end(), 0.0);

  return true;
}

}  // namespace sptk
