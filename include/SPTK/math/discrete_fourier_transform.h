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

#ifndef SPTK_MATH_DISCRETE_FOURIER_TRANSFORM_H_
#define SPTK_MATH_DISCRETE_FOURIER_TRANSFORM_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Calculate DFT of complex-valued input data.
 *
 * The inputs are @f$L@f$-length complex-valued data:
 * @f[
 *   \begin{array}{cccc}
 *   \mathrm{Re}(x(0)), & \mathrm{Re}(x(1)), & \ldots, & \mathrm{Re}(x(L-1)), \\
 *   \mathrm{Im}(x(0)), & \mathrm{Im}(x(1)), & \ldots, & \mathrm{Im}(x(L-1)).
 *   \end{array}
 * @f]
 * The outputs are
 * @f[
 *   \begin{array}{cccc}
 *   \mathrm{Re}(X(0)), & \mathrm{Re}(X(1)), & \ldots, & \mathrm{Re}(X(L-1)), \\
 *   \mathrm{Im}(X(0)), & \mathrm{Im}(X(1)), & \ldots, & \mathrm{Im}(X(L-1)).
 *   \end{array}
 * @f]
 * They are computed as
 * @f[
 *   X(k) = \sum_{n=0}^{L-1} x(n) e^{-j2\pi nk / L}.
 * @f]
 */
class DiscreteFourierTransform {
 public:
  /**
   * @param[in] dft_length DFT length, @f$L@f$.
   */
  explicit DiscreteFourierTransform(int dft_length);

  virtual ~DiscreteFourierTransform() {
  }

  /**
   * @return DFT length.
   */
  int GetDftLength() const {
    return dft_length_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] real_part_input @f$L@f$-length real part of input.
   * @param[in] imag_part_input @f$L@f$-length imaginary part of input.
   * @param[out] real_part_output @f$L@f$-length real part of output.
   * @param[out] imag_part_output @f$L@f$-length imaginary part of output.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& real_part_input,
           const std::vector<double>& imag_part_input,
           std::vector<double>* real_part_output,
           std::vector<double>* imag_part_output) const;

  /**
   * @param[in,out] real_part @f$L@f$-length real part.
   * @param[in,out] imag_part @f$L@f$-length imaginary part.
   * @return True on success, false on failure.
   */
  bool Run(std::vector<double>* real_part,
           std::vector<double>* imag_part) const;

 private:
  const int dft_length_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(DiscreteFourierTransform);
};

}  // namespace sptk

#endif  // SPTK_MATH_DISCRETE_FOURIER_TRANSFORM_H_
