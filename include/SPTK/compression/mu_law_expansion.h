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

#ifndef SPTK_COMPRESSION_MU_LAW_EXPANSION_H_
#define SPTK_COMPRESSION_MU_LAW_EXPANSION_H_

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Nonlinearly decompress data based on @f$\mu@f$-law algorithm.
 *
 * Given the input data @f$y(n)@f$, the expansion is performed as follows:
 * @f[
 *   x(n) = V \, \mathrm{sgn}(y(n))
 *     \frac{(1 + \mu)^{|y(n)|/V} - 1}{\mu}
 * @f]
 * where @f$V@f$ is the absolute maximum value of the input data and @f$\mu@f$
 * is the compression factor, which is typically set to 255.
 */
class MuLawExpansion {
 public:
  /**
   * @param[in] abs_max_value Absolute maximum value.
   * @param[in] compression_factor Compression factor, @f$\mu@f$.
   */
  MuLawExpansion(double abs_max_value, double compression_factor);

  virtual ~MuLawExpansion() {
  }

  /**
   * @return Absolute maximum value.
   */
  double GetAbsMaxValue() const {
    return abs_max_value_;
  }

  /**
   * @return Compression factor.
   */
  double GetCompressionFactor() const {
    return compression_factor_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] input Input data.
   * @param[out] output Output data.
   * @return True on success, false on failure.
   */
  bool Run(double input, double* output) const;

  /**
   * @param[in,out] input_and_output Input/output data.
   * @return True on success, false on failure.
   */
  bool Run(double* input_and_output) const;

 private:
  const double abs_max_value_;
  const double compression_factor_;
  const double constant_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(MuLawExpansion);
};

}  // namespace sptk

#endif  // SPTK_COMPRESSION_MU_LAW_EXPANSION_H_
