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

#include "SPTK/math/histogram_calculation.h"

#include <algorithm>  // std::fill
#include <cmath>      // std::floor
#include <cstddef>    // std::size_t

namespace sptk {

HistogramCalculation::HistogramCalculation(int num_bin, double lower_bound,
                                           double upper_bound)
    : num_bin_(num_bin),
      lower_bound_(lower_bound),
      upper_bound_(upper_bound),
      bin_width_((upper_bound_ - lower_bound_) / num_bin_),
      is_valid_(true) {
  if (num_bin_ <= 0 || upper_bound_ <= lower_bound_) {
    is_valid_ = false;
    return;
  }
}

bool HistogramCalculation::Run(const std::vector<double>& data,
                               std::vector<double>* histogram) const {
  // Check inputs.
  if (!is_valid_ || data.empty() || NULL == histogram) {
    return false;
  }

  // Prepare memories.
  if (histogram->size() != static_cast<std::size_t>(num_bin_)) {
    histogram->resize(num_bin_);
  }

  std::fill(histogram->begin(), histogram->end(), 0.0);

  const int length(static_cast<int>(data.size()));
  const double* input(&(data[0]));
  double* output(&((*histogram)[0]));
  for (int i(0); i < length; ++i) {
    if (lower_bound_ <= input[i] && input[i] < upper_bound_) {
      const int bin_index(
          static_cast<int>(std::floor((input[i] - lower_bound_) / bin_width_)));
      ++output[bin_index];
    } else if (upper_bound_ == input[i]) {
      ++output[num_bin_ - 1];
    }
  }

  return true;
}

}  // namespace sptk
