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

#ifndef SPTK_MATH_HISTOGRAM_CALCULATION_H_
#define SPTK_MATH_HISTOGRAM_CALCULATION_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Calculate histogram.
 *
 * The input is a data sequence:
 * @f[
 *   \begin{array}{ccc}
 *     x(0), & x(1), & \ldots,
 *   \end{array}
 * @f]
 * and the output is the histogram of data:
 * @f[
 *   \begin{array}{cccc}
 *     b(1), & b(2), & \ldots, & b(N),
 *   \end{array}
 * @f]
 * where @f$N@f$ is the number of bins.
 *
 * The width of the bin is calculated as
 * @f[
 *    \frac{y_U - y_L}{N},
 * @f]
 * where @f$y_U@f$ and @f$y_L@f$ are the upper bound and the lower bound of
 * the histogram. The data that satisfies @f$x(t) > y_U@f$ or
 * @f$x(t) < y_L@f$ is not contributed to any bins.
 */
class HistogramCalculation {
 public:
  /**
   * @param[in] num_bin Number of bins, @f$N@f$.
   * @param[in] lower_bound Lower bound, @f$y_L@f$.
   * @param[in] upper_bound Upper bound, @f$y_U@f$.
   */
  HistogramCalculation(int num_bin, double lower_bound, double upper_bound);

  virtual ~HistogramCalculation() {
  }

  /**
   * @return Number of bins.
   */
  int GetNumBin() const {
    return num_bin_;
  }

  /**
   * @return Lower bound.
   */
  double GetLowerBound() const {
    return lower_bound_;
  }

  /**
   * @return Upper bound.
   */
  double GetUpperBound() const {
    return upper_bound_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] data Input data.
   * @param[out] histogram Histogram.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& data,
           std::vector<double>* histogram) const;

 private:
  const int num_bin_;
  const double lower_bound_;
  const double upper_bound_;
  const double bin_width_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(HistogramCalculation);
};

}  // namespace sptk

#endif  // SPTK_MATH_HISTOGRAM_CALCULATION_H_
