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

#ifndef SPTK_CONVERSION_ALL_POLE_TO_ALL_ZERO_DIGITAL_FILTER_COEFFICIENTS_H_
#define SPTK_CONVERSION_ALL_POLE_TO_ALL_ZERO_DIGITAL_FILTER_COEFFICIENTS_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Convert all-pole to all-zero digital filter coefficents vice versa.
 *
 * The input is the @f$M@f$-th order filter coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     K, & a(1), & \ldots, & a(M),
 *   \end{array}
 * @f]
 * and the output is
 * @f[
 *   \begin{array}{cccc}
 *     b(0), & b(1), & \ldots, & b(M),
 *   \end{array}
 * @f]
 * where
 * @f[
 *   b(m) = \left\{ \begin{array}{ll}
 *     1/K, & m = 0 \\
 *     a(m)/K. & 1 \le m \le M
 *   \end{array} \right.
 * @f]
 *
 * The transfer function of an all-pole digital filter is
 * @f[
 *   H(z) = \frac{K}{1 + \displaystyle\sum_{m=1}^M a(m) z^{-m}}.
 * @f]
 * The inverse filter can be written as
 * @f{eqnarray}{
 *   \frac{1}{H(z)} &=& \frac{1 + \displaystyle\sum_{m=1}^M a(m) z^{-m}}{K} \\
 *                  &=& \frac{1}{K} + \sum_{m=1}^M \frac{a(m)}{K} z^{-m} \\
 *                  &=& \sum_{m=0}^M b(m) z^{-m}.
 * @f}
 * The conversion is symmetric.
 */
class AllPoleToAllZeroDigitalFilterCoefficients {
 public:
  /**
   * @param[in] num_order Order of coefficients, @f$M@f$.
   */
  explicit AllPoleToAllZeroDigitalFilterCoefficients(int num_order);

  virtual ~AllPoleToAllZeroDigitalFilterCoefficients() {
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
   * @param[in] input_filter_coefficients @f$M@f$-th order filter coefficients.
   * @param[out] output_filter_coefficients Converted @f$M@f$-th order filter
   *             coefficients.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& input_filter_coefficients,
           std::vector<double>* output_filter_coefficients) const;

  /**
   * @param[in,out] input_and_output @f$M@f$-th order filter coefficients.
   * @return True on success, false on failure.
   */
  bool Run(std::vector<double>* input_and_output) const;

 private:
  const int num_order_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(AllPoleToAllZeroDigitalFilterCoefficients);
};

}  // namespace sptk

#endif  // SPTK_CONVERSION_ALL_POLE_TO_ALL_ZERO_DIGITAL_FILTER_COEFFICIENTS_H_
