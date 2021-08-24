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

#ifndef SPTK_MATH_DISTANCE_CALCULATION_H_
#define SPTK_MATH_DISTANCE_CALCULATION_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Calculate distance between two vectors.
 */
class DistanceCalculation {
 public:
  /**
   * Metric of distance
   */
  enum DistanceMetrics {
    kManhattan = 0,
    kEuclidean,
    kSquaredEuclidean,
    kSymmetricKullbackLeibler,
    kNumMetrics
  };

  /**
   * @param[in] num_order Order of vector, @f$M@f$.
   * @param[in] distance_metric Distance metric.
   */
  DistanceCalculation(int num_order, DistanceMetrics distance_metric);

  virtual ~DistanceCalculation() {
  }

  /**
   * @return Order of vector.
   */
  int GetNumOrder() const {
    return num_order_;
  }

  /**
   * @return Distance metric.
   */
  DistanceMetrics GetDistanceMetric() const {
    return distance_metric_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] vector1 @f$M@f$-th order vector.
   * @param[in] vector2 @f$M@f$-th order vector.
   * @param[out] distance Distance between the two vectors.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& vector1,
           const std::vector<double>& vector2, double* distance) const;

 private:
  const int num_order_;
  const DistanceMetrics distance_metric_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(DistanceCalculation);
};

}  // namespace sptk

#endif  // SPTK_MATH_DISTANCE_CALCULATION_H_
