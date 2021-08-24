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

#ifndef SPTK_CONVERSION_LOG_AREA_RATIO_TO_PARCOR_COEFFICIENTS_H_
#define SPTK_CONVERSION_LOG_AREA_RATIO_TO_PARCOR_COEFFICIENTS_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Convert LAR coefficients to PARCOR coefficients.
 *
 * The input is the @f$M@f$-th order LAR coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     K, & g(1), & \ldots, & g(M),
 *   \end{array}
 * @f]
 * and the output is the @f$M@f$-th order PARCOR coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     K, & k(1), & \ldots, & k(M),
 *   \end{array}
 * @f]
 * where @f$K@f$ is the gain. The conversion is performed by
 * @f{eqnarray}{
 *   k(m) &=& \frac{e^{g(m)} - 1}{e^{g(m)} + 1}\\
 *        &=& \tanh(g(m)/2).
 * @f}
 */
class LogAreaRatioToParcorCoefficients {
 public:
  /**
   * @param[in] num_order Order of coefficients.
   */
  explicit LogAreaRatioToParcorCoefficients(int num_order);

  virtual ~LogAreaRatioToParcorCoefficients() {
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
   * @param[in] log_area_ratio @f$M@f$-th order LAR coefficients.
   * @param[out] parcor_coefficients @f$M@f$-th order PARCOR coefficients.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& log_area_ratio,
           std::vector<double>* parcor_coefficients) const;

  /**
   * @param[in,out] input_and_output @f$M@f$-th order coefficients.
   * @return True on success, false on failure.
   */
  bool Run(std::vector<double>* input_and_output) const;

 private:
  const int num_order_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(LogAreaRatioToParcorCoefficients);
};

}  // namespace sptk

#endif  // SPTK_CONVERSION_LOG_AREA_RATIO_TO_PARCOR_COEFFICIENTS_H_
