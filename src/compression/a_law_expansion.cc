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

#include "SPTK/compression/a_law_expansion.h"

#include <cmath>  // std::exp, std::fabs, std::log

namespace sptk {

ALawExpansion::ALawExpansion(double abs_max_value, double compression_factor)
    : abs_max_value_(abs_max_value),
      compression_factor_(compression_factor),
      constant_(abs_max_value_ / compression_factor_),
      is_valid_(true) {
  if (abs_max_value_ <= 0.0 || compression_factor_ < 1.0) {
    is_valid_ = false;
    return;
  }
}

bool ALawExpansion::Run(double input, double* output) const {
  if (!is_valid_ || NULL == output) {
    return false;
  }

  const double x(std::fabs(input) / abs_max_value_);
  const double z(1 + std::log(compression_factor_));
  double y(z * x);
  if (1.0 / z <= x) {
    y = std::exp(y - 1.0);
  }
  *output = constant_ * sptk::ExtractSign(input) * y;

  return true;
}

bool ALawExpansion::Run(double* input_and_output) const {
  if (NULL == input_and_output) return false;
  return Run(*input_and_output, input_and_output);
}

}  // namespace sptk
