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

#ifndef SPTK_MATH_INVERSE_DISCRETE_COSINE_TRANSFORM_H_
#define SPTK_MATH_INVERSE_DISCRETE_COSINE_TRANSFORM_H_

#include <vector>  // std::vector

#include "SPTK/math/fourier_transform.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Calculate inverse DCT-II of complex-valued input data.
 *
 * The inputs are @f$L@f$-length order complex-valued data:
 * @f[
 *   \begin{array}{cccc}
 *   \mathrm{Re}(X(0)), & \mathrm{Re}(X(1)), & \ldots, & \mathrm{Re}(X(L-1)), \\
 *   \mathrm{Im}(X(0)), & \mathrm{Im}(X(1)), & \ldots, & \mathrm{Im}(X(L-1)).
 *   \end{array}
 * @f]
 * The outputs are
 * @f[
 *   \begin{array}{cccc}
 *   \mathrm{Re}(x(0)), & \mathrm{Re}(x(1)), & \ldots, & \mathrm{Re}(x(L-1)), \\
 *   \mathrm{Im}(x(0)), & \mathrm{Im}(x(1)), & \ldots, & \mathrm{Im}(x(L-1)).
 *   \end{array}
 * @f]
 * They are computed as
 * @f[
 *   x(n) = \sqrt{\frac{2}{L}} \sum_{k=0}^{L-1} c(k) X(k) \cos
 *     \left( \frac{\pi}{L} \left( n + \frac{1}{2} \right) k \right),
 * @f]
 * where
 * @f[
 *   c(k) = \left\{ \begin{array}{ll}
 *     1/\sqrt{2}, & k = 0 \\
 *     1. & 1 \le k < L
 *   \end{array} \right.
 * @f]
 */
class InverseDiscreteCosineTransform {
 public:
  /**
   * Buffer for InverseDiscreteCosineTransform class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    std::vector<double> fourier_transform_real_part_;
    std::vector<double> fourier_transform_imag_part_;

    friend class InverseDiscreteCosineTransform;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] dct_length DCT length, @f$L@f$.
   */
  explicit InverseDiscreteCosineTransform(int dct_length);

  virtual ~InverseDiscreteCosineTransform() {
  }

  /**
   * @return DCT length.
   */
  int GetDctLength() const {
    return dct_length_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return fourier_transform_.IsValid();
  }

  /**
   * @param[in] real_part_input @f$L@f$-length real part of input.
   * @param[in] imag_part_input @f$L@f$-length imaginary part of input.
   * @param[out] real_part_output @f$L@f$-length real part of output.
   * @param[out] imag_part_output @f$L@f$-length imaginary part of output.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& real_part_input,
           const std::vector<double>& imag_part_input,
           std::vector<double>* real_part_output,
           std::vector<double>* imag_part_output,
           InverseDiscreteCosineTransform::Buffer* buffer) const;

  /**
   * @param[in,out] real_part @f$L@f$-length real part.
   * @param[in,out] imag_part @f$L@f$-length imaginary part.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(std::vector<double>* real_part, std::vector<double>* imag_part,
           InverseDiscreteCosineTransform::Buffer* buffer) const;

 private:
  const int dct_length_;

  const FourierTransform fourier_transform_;

  std::vector<double> cosine_table_;
  std::vector<double> sine_table_;

  DISALLOW_COPY_AND_ASSIGN(InverseDiscreteCosineTransform);
};

}  // namespace sptk

#endif  // SPTK_MATH_INVERSE_DISCRETE_COSINE_TRANSFORM_H_
