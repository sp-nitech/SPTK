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

#ifndef SPTK_CONVERSION_COMPOSITE_SINUSOIDAL_MODELING_TO_AUTOCORRELATION_H_
#define SPTK_CONVERSION_COMPOSITE_SINUSOIDAL_MODELING_TO_AUTOCORRELATION_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Convert CSM parameters to autocorrelation.
 *
 * The input is the @f$2N@f$ CSM parameters:
 * @f[
 *   \begin{array}{cccc}
 *     \omega(1), & \omega(2), & \ldots, & \omega(N), \\
 *     m(1), & m(2), & \ldots, & m(N),
 *   \end{array}
 * @f]
 * where @f$\omega(n)@f$ is the CSM frequency and @f$m(n)@f$ is the CSM
 * intensity. The output is the @f$(2N-1)@f$-th order biased sample
 * autocorrelation coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     v(0), & v(1), & \ldots, & v(2N-1).
 *   \end{array}
 * @f]
 * The sample autocorrelation is given by
 * @f[
 *     v(l) = \sum_{i=1}^N m(i) \cos(l \, \omega(i)).
 * @f]
 *
 * [1] S. Sagayama and F. Itakura, &quot;Duality theory of composite sinusoidal
 *     modeling and linear prediction,&quot; Proc. of ICASSP 1986,
 *     pp. 1261-1264, 1986.
 */
class CompositeSinusoidalModelingToAutocorrelation {
 public:
  /**
   * @param[in] num_sine_wave Number of sine waves, @f$N@f$.
   */
  explicit CompositeSinusoidalModelingToAutocorrelation(int num_sine_wave);

  virtual ~CompositeSinusoidalModelingToAutocorrelation() {
  }

  /**
   * @return Number of sine waves.
   */
  int GetNumSineWaves() const {
    return num_sine_wave_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] composite_sinusoidal_modeling CSM parameters.
   * @param[out] autocorrelation Autocorrelation coefficients.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& composite_sinusoidal_modeling,
           std::vector<double>* autocorrelation) const;

  /**
   * @param[in,out] input_and_output Input and output.
   * @return True on success, false on failure.
   */
  bool Run(std::vector<double>* input_and_output) const;

 private:
  const int num_sine_wave_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(CompositeSinusoidalModelingToAutocorrelation);
};

}  // namespace sptk

#endif  // SPTK_CONVERSION_COMPOSITE_SINUSOIDAL_MODELING_TO_AUTOCORRELATION_H_
