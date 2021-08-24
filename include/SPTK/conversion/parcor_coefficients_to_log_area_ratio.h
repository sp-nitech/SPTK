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

#ifndef SPTK_CONVERSION_PARCOR_COEFFICIENTS_TO_LOG_AREA_RATIO_H_
#define SPTK_CONVERSION_PARCOR_COEFFICIENTS_TO_LOG_AREA_RATIO_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Convert PARCOR coefficients to log area ratio (LAR).
 *
 * The input is the @f$M@f$-th order PARCOR coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     K, & k(1), & \ldots, & k(M),
 *   \end{array}
 * @f]
 * and the output is the @f$M@f$-th order LAR coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     K, & g(1), & \ldots, & g(M),
 *   \end{array}
 * @f]
 * where @f$K@f$ is the gain. The conversion is performed by
 * @f{eqnarray}{
 *   g(m) &=& \frac{1 + k(m)}{1 - k(m)} \\
 *        &=& 2\tanh^{-1}(k(m)).
 * @f}
 */
class ParcorCoefficientsToLogAreaRatio {
 public:
  /**
   * @param[in] num_order Order of coefficients.
   */
  explicit ParcorCoefficientsToLogAreaRatio(int num_order);

  virtual ~ParcorCoefficientsToLogAreaRatio() {
  }

  /**
   * @return Order of coefficients.
   */
  int GetNumOrder() const {
    return num_order_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] parcor_coefficients @f$M@f$-th order PARCOR coefficients.
   * @param[out] log_area_ratio @f$M@f$-th order LAR coefficients.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& parcor_coefficients,
           std::vector<double>* log_area_ratio) const;

  /**
   * @param[in,out] input_and_output @f$M@f$-th order coefficients.
   * @return True on success, false on failure.
   */
  bool Run(std::vector<double>* input_and_output) const;

 private:
  const int num_order_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(ParcorCoefficientsToLogAreaRatio);
};

}  // namespace sptk

#endif  // SPTK_CONVERSION_PARCOR_COEFFICIENTS_TO_LOG_AREA_RATIO_H_
