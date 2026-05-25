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

#ifndef SPTK_MATH_LP_NORM_CALCULATION_H_
#define SPTK_MATH_LP_NORM_CALCULATION_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Calculate Lp-norm.
 *
 * The input is the @f$M@f$-th order vector:
 * @f[
 *   \begin{array}{cccc}
 *     x(0), & x(1), & \ldots, & x(M),
 *   \end{array}
 * @f]
 * The output is the Lp-norm represented as
 * @f[
 *   \begin{array}{cccc}
 *     \|x\|_p = \left(\displaystyle\sum_{m=0}^M |x(m)|^p\right)^{\frac{1}{p}}.
 *   \end{array}
 * @f]
 */
class LpNormCalculation {
 public:
  /**
   * @param[in] num_order Order of input, @f$M@f$.
   * @param[in] p Power, @f$p@f$.
   */
  LpNormCalculation(int num_order, double p);

  virtual ~LpNormCalculation() {
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] data @f$M@f$-th order vector.
   * @param[out] norm Lp-norm.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& data, double* norm) const;

 private:
  const int num_order_;
  const double p_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(LpNormCalculation);
};

}  // namespace sptk

#endif  // SPTK_MATH_LP_NORM_CALCULATION_H_
