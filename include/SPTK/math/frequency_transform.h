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

#ifndef SPTK_MATH_FREQUENCY_TRANSFORM_H_
#define SPTK_MATH_FREQUENCY_TRANSFORM_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Transform a minimum phase sequence into a frequency-warped sequence.
 *
 * The input is the @f$M_1@f$-th order minimum phase sequence:
 * @f[
 *   \begin{array}{cccc}
 *     c_{\alpha_1}(0), & c_{\alpha_1}(1), & \ldots, & c_{\alpha_1}(M_1),
 *   \end{array}
 * @f]
 * and the output is the @f$M_2@f$-th order frequency-warped sequence:
 * @f[
 *   \begin{array}{cccc}
 *     c_{\alpha_2}(0), & c_{\alpha_2}(1), & \ldots, & c_{\alpha_2}(M_2).
 *   \end{array}
 * @f]
 * The output sequence can be obtained by using the following recursion formula:
 * @f[
 *   c_{\alpha_2}^{(i)}(m) = \left\{\begin{array}{ll}
 *     c_{\alpha_1}(-i) + \alpha\,c_{\alpha_2}^{(i-1)}(0), & m=0 \\
 *     (1-\alpha^2)\,c_{\alpha_2}^{(i-1)}(0) +
 *       \alpha\,c_{\alpha_2}^{(i-1)}(1), & m=1 \\
 *     c_{\alpha_2}^{(i-1)}(m-1) + \alpha (c_{\alpha_2}^{(i-1)}(m) -
 *       c_{\alpha_2}^{(i)}(m-1)), & m=2,3,\ldots,M_2
 *   \end{array} \right. \\
 *   i = -M_1,\ldots,-1,0
 * @f]
 * where
 * @f[
 *   \alpha = (\alpha_2 - \alpha_1)\,/\,(1 - \alpha_1 \alpha_2).
 * @f]
 * The initial condition of the recursion is
 * @f$c_{\alpha_2}^{(-M_1-1)}(m) = 0@f$ for any @f$m@f$.
 *
 * The transformation is based on the cascade of all-pass networks. For more
 * detail, see [1]. Note that the above recursion can be represented as a linear
 * transformation, i.e., matrix multiplication.
 *
 * [1] A. Oppenheim and D. Johnson, &quot;Discrete representation of
 *     signals,&quot; Proc. of the IEEE, vol. 60, no. 6, pp. 681-691, 1972.
 *
 * [2] K. Tokuda, T. Kobayashi, T. Masuko, and S. Imai, &quot;Mel-generalized
 *     cepstral representation of speech - A unified approach to speech spectral
 *     estimation,&quot; Proc. of ICSLP 1994, pp. 1043-1046, 1994.
 */
class FrequencyTransform {
 public:
  /**
   * Buffer for FrequencyTransform class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    std::vector<double> d_;

    friend class FrequencyTransform;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_input_order Order of input, @f$M_1@f$.
   * @param[in] num_output_order Order of output, @f$M_2@f$.
   * @param[in] alpha Frequency warping factor, @f$\alpha@f$.
   */
  FrequencyTransform(int num_input_order, int num_output_order, double alpha);

  virtual ~FrequencyTransform() {
  }

  /**
   * @return Order of input.
   */
  int GetNumInputOrder() const {
    return num_input_order_;
  }

  /**
   * @return Order of output.
   */
  int GetNumOutputOrder() const {
    return num_output_order_;
  }

  /**
   * @return Frequency warping factor.
   */
  double GetAlpha() const {
    return alpha_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] minimum_phase_sequence @f$M_1@f$-th order input sequence.
   * @param[out] warped_sequence @f$M_2@f$-th order output sequence.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& minimum_phase_sequence,
           std::vector<double>* warped_sequence,
           FrequencyTransform::Buffer* buffer) const;

 private:
  const int num_input_order_;
  const int num_output_order_;
  const double alpha_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(FrequencyTransform);
};

}  // namespace sptk

#endif  // SPTK_MATH_FREQUENCY_TRANSFORM_H_
