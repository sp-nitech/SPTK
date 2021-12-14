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

#include "SPTK/input/input_source_preprocessing_for_filter_gain.h"

#include <algorithm>  // std::transform
#include <cmath>      // std::exp

namespace sptk {

InputSourcePreprocessingForFilterGain::InputSourcePreprocessingForFilterGain(
    FilterGainType gain_type, InputSourceInterface* source)
    : gain_type_(gain_type), source_(source), is_valid_(true) {
  if (NULL == source_ || !source_->IsValid()) {
    is_valid_ = false;
    return;
  }
}

bool InputSourcePreprocessingForFilterGain::Get(std::vector<double>* buffer) {
  if (NULL == buffer || !is_valid_) {
    return false;
  }

  if (!source_->Get(buffer)) {
    return false;
  }

  switch (gain_type_) {
    case kLinear: {
      // nothing to do
      break;
    }
    case kLog: {
      (*buffer)[0] = std::exp((*buffer)[0]);
      break;
    }
    case kUnity: {
      (*buffer)[0] = 1.0;
      break;
    }
    case kUnityForAllZeroFilter: {
      if (0.0 == (*buffer)[0]) return false;
      const double inverse_of_b0(1.0 / (*buffer)[0]);
      std::transform(buffer->begin(), buffer->end(), buffer->begin(),
                     [inverse_of_b0](double x) { return x * inverse_of_b0; });
      break;
    }
    default: {
      return false;
    }
  }

  return true;
}

}  // namespace sptk
