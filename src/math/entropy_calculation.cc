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

#include "SPTK/math/entropy_calculation.h"

#include <cstddef>  // std::size_t

namespace sptk {

EntropyCalculation::EntropyCalculation(int num_element,
                                       EntropyUnits entropy_unit)
    : num_element_(num_element), entropy_unit_(entropy_unit), is_valid_(true) {
  if (num_element_ <= 0 || kNumUnits == entropy_unit_) {
    is_valid_ = false;
    return;
  }
}

bool EntropyCalculation::Run(const std::vector<double>& probability,
                             double* entropy) const {
  if (!is_valid_ ||
      probability.size() != static_cast<std::size_t>(num_element_) ||
      NULL == entropy) {
    return false;
  }

  const double* p(&(probability[0]));
  double sum(0.0);

  switch (entropy_unit_) {
    case kBit: {
      for (int i(0); i < num_element_; ++i) {
        sum += p[i] * FloorLog2(p[i]);
      }
      break;
    }
    case kNat: {
      for (int i(0); i < num_element_; ++i) {
        sum += p[i] * FloorLog(p[i]);
      }
      break;
    }
    case kDit: {
      for (int i(0); i < num_element_; ++i) {
        sum += p[i] * FloorLog10(p[i]);
      }
      break;
    }
    default: {
      return false;
    }
  }

  *entropy = -sum;

  return true;
}

}  // namespace sptk
