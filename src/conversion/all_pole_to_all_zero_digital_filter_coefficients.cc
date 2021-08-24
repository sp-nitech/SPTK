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

#include "SPTK/conversion/all_pole_to_all_zero_digital_filter_coefficients.h"

#include <algorithm>  // std::transform
#include <cstddef>    // std::size_t

namespace sptk {

AllPoleToAllZeroDigitalFilterCoefficients::
    AllPoleToAllZeroDigitalFilterCoefficients(int num_order)
    : num_order_(num_order), is_valid_(true) {
  if (num_order_ < 0) {
    is_valid_ = false;
    return;
  }
}

bool AllPoleToAllZeroDigitalFilterCoefficients::Run(
    const std::vector<double>& input_filter_coefficients,
    std::vector<double>* output_filter_coefficients) const {
  // Check inputs.
  const int length(num_order_ + 1);
  if (!is_valid_ ||
      input_filter_coefficients.size() != static_cast<std::size_t>(length) ||
      0.0 == input_filter_coefficients[0] ||
      NULL == output_filter_coefficients) {
    return false;
  }

  // Prepare memories.
  if (output_filter_coefficients->size() != static_cast<std::size_t>(length)) {
    output_filter_coefficients->resize(length);
  }

  // Perform conversion.
  const double z(1.0 / input_filter_coefficients[0]);
  (*output_filter_coefficients)[0] = z;
  std::transform(
      input_filter_coefficients.begin() + 1, input_filter_coefficients.end(),
      output_filter_coefficients->begin() + 1, [z](double x) { return x * z; });

  return true;
}

bool AllPoleToAllZeroDigitalFilterCoefficients::Run(
    std::vector<double>* input_and_output) const {
  if (NULL == input_and_output) return false;
  std::vector<double> input(*input_and_output);
  return Run(input, input_and_output);
}

}  // namespace sptk
