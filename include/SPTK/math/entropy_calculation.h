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

#ifndef SPTK_MATH_ENTROPY_CALCULATION_H_
#define SPTK_MATH_ENTROPY_CALCULATION_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Calculate entropy.
 *
 * The input is the probabilities of @f$N@f$ events:
 * @f[
 *   \begin{array}{cccc}
 *     p(1), & p(2), & \ldots, & p(N),
 *   \end{array}
 * @f]
 * where
 * @f[
 *   \sum_{n=1}^N \, p(n) = 1.
 * @f]
 * The output is the entropy for the probabilities:
 * @f[
 *   \begin{array}{cccc}
 *     H = -\displaystyle\sum_{n=1}^N p(n) \log_b p(n)
 *   \end{array}
 * @f]
 * where @f$b@f$ is 2, @f$e@f$, or 10.
 */
class EntropyCalculation {
 public:
  /**
   * Unit of entropy.
   */
  enum EntropyUnits { kBit = 0, kNat, kDit, kNumUnits };

  /**
   * @param[in] num_element Number of elements, @f$N@f$.
   * @param[in] entropy_unit Unit of entropy.
   */
  EntropyCalculation(int num_element, EntropyUnits entropy_unit);

  virtual ~EntropyCalculation() {
  }

  /**
   * @return Number of elements.
   */
  int GetNumElement() const {
    return num_element_;
  }

  /**
   * @return Unit of entropy.
   */
  EntropyUnits GetEntropyUnit() const {
    return entropy_unit_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] probability Probabiltiy distribution.
   * @param[out] entropy Entropy.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& probability, double* entropy) const;

 private:
  const int num_element_;
  const EntropyUnits entropy_unit_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(EntropyCalculation);
};

}  // namespace sptk

#endif  // SPTK_MATH_ENTROPY_CALCULATION_H_
