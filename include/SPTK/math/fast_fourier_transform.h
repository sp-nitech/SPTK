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

#ifndef SPTK_MATH_FAST_FOURIER_TRANSFORM_H_
#define SPTK_MATH_FAST_FOURIER_TRANSFORM_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Calculate FFT of complex-valued input data.
 *
 * The inputs are @f$M@f$-th order complex-valued data:
 * @f[
 *   \begin{array}{cccc}
 *   \mathrm{Re}(x(0)), & \mathrm{Re}(x(1)), & \ldots, & \mathrm{Re}(x(M)), \\
 *   \mathrm{Im}(x(0)), & \mathrm{Im}(x(1)), & \ldots, & \mathrm{Im}(x(M)).
 *   \end{array}
 * @f]
 * The outputs are
 * @f[
 *   \begin{array}{cccc}
 *   \mathrm{Re}(X(0)), & \mathrm{Re}(X(1)), & \ldots, & \mathrm{Re}(X(L-1)), \\
 *   \mathrm{Im}(X(0)), & \mathrm{Im}(X(1)), & \ldots, & \mathrm{Im}(X(L-1)),
 *   \end{array}
 * @f]
 * where @f$L@f$ is the FFT length and must be a power of two.
 */
class FastFourierTransform {
 public:
  /**
   * @param[in] fft_length FFT length, @f$L@f$.
   */
  explicit FastFourierTransform(int fft_length);

  /**
   * @param[in] num_order Order of input, @f$M@f$.
   * @param[in] fft_length FFT length, @f$L@f$.
   */
  FastFourierTransform(int num_order, int fft_length);

  virtual ~FastFourierTransform() {
  }

  /**
   * @return Order of input.
   */
  int GetNumOrder() const {
    return num_order_;
  }

  /**
   * @return FFT length.
   */
  int GetFftLength() const {
    return fft_length_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] real_part_input @f$M@f$-th order real part of input.
   * @param[in] imag_part_input @f$M@f$-th order imaginary part of input.
   * @param[out] real_part_output @f$L@f$-length real part of output.
   * @param[out] imag_part_output @f$L@f$-length imaginary part of output.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& real_part_input,
           const std::vector<double>& imag_part_input,
           std::vector<double>* real_part_output,
           std::vector<double>* imag_part_output) const;

  /**
   * @param[in,out] real_part Real part.
   * @param[in,out] imag_part Imaginary part.
   * @return True on success, false on failure.
   */
  bool Run(std::vector<double>* real_part,
           std::vector<double>* imag_part) const;

 private:
  const int num_order_;
  const int fft_length_;
  const int half_fft_length_;

  bool is_valid_;

  std::vector<double> sine_table_;

  DISALLOW_COPY_AND_ASSIGN(FastFourierTransform);
};

}  // namespace sptk

#endif  // SPTK_MATH_FAST_FOURIER_TRANSFORM_H_
