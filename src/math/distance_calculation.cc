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

#include "SPTK/math/distance_calculation.h"

#include <cmath>    // std::fabs, std::log, std::sqrt
#include <cstddef>  // std::size_t

namespace sptk {

DistanceCalculation::DistanceCalculation(int num_order,
                                         DistanceMetrics distance_metric)
    : num_order_(num_order),
      distance_metric_(distance_metric),
      is_valid_(true) {
  if (num_order_ < 0 || kNumMetrics == distance_metric_) {
    is_valid_ = false;
    return;
  }
}

bool DistanceCalculation::Run(const std::vector<double>& vector1,
                              const std::vector<double>& vector2,
                              double* distance) const {
  // Check inputs.
  if (!is_valid_ ||
      vector1.size() != static_cast<std::size_t>(num_order_ + 1) ||
      vector2.size() != static_cast<std::size_t>(num_order_ + 1) ||
      NULL == distance) {
    return false;
  }

  const double* x(&(vector1[0]));
  const double* y(&(vector2[0]));

  double sum(0.0);

  switch (distance_metric_) {
    case kManhattan: {
      for (int m(0); m <= num_order_; ++m) {
        const double diff(x[m] - y[m]);
        sum += std::fabs(diff);
      }
      break;
    }
    case kEuclidean: {
      for (int m(0); m <= num_order_; ++m) {
        const double diff(x[m] - y[m]);
        sum += diff * diff;
      }
      sum = std::sqrt(sum);
      break;
    }
    case kSquaredEuclidean: {
      for (int m(0); m <= num_order_; ++m) {
        const double diff(x[m] - y[m]);
        sum += diff * diff;
      }
      break;
    }
    case kSymmetricKullbackLeibler: {
      for (int m(0); m <= num_order_; ++m) {
        if (x[m] <= 0.0 || y[m] <= 0.0) return false;
        const double diff(x[m] - y[m]);
        const double log_diff(std::log(x[m]) - std::log(y[m]));
        sum += diff * log_diff;
      }
      break;
    }
    default: {
      return false;
    }
  }

  *distance = sum;

  return true;
}

}  // namespace sptk
