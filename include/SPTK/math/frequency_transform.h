// ----------------------------------------------------------------- //
//             The Speech Signal Processing Toolkit (SPTK)           //
//             developed by SPTK Working Group                       //
//             http://sp-tk.sourceforge.net/                         //
// ----------------------------------------------------------------- //
//                                                                   //
//  Copyright (c) 1984-2007  Tokyo Institute of Technology           //
//                           Interdisciplinary Graduate School of    //
//                           Science and Engineering                 //
//                                                                   //
//                1996-2020  Nagoya Institute of Technology          //
//                           Department of Computer Science          //
//                                                                   //
// All rights reserved.                                              //
//                                                                   //
// Redistribution and use in source and binary forms, with or        //
// without modification, are permitted provided that the following   //
// conditions are met:                                               //
//                                                                   //
// - Redistributions of source code must retain the above copyright  //
//   notice, this list of conditions and the following disclaimer.   //
// - Redistributions in binary form must reproduce the above         //
//   copyright notice, this list of conditions and the following     //
//   disclaimer in the documentation and/or other materials provided //
//   with the distribution.                                          //
// - Neither the name of the SPTK working group nor the names of its //
//   contributors may be used to endorse or promote products derived //
//   from this software without specific prior written permission.   //
//                                                                   //
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            //
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       //
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          //
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          //
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS //
// BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          //
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   //
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     //
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON //
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   //
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    //
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           //
// POSSIBILITY OF SUCH DAMAGE.                                       //
// ----------------------------------------------------------------- //

#ifndef SPTK_MATH_FREQUENCY_TRANSFORM_H_
#define SPTK_MATH_FREQUENCY_TRANSFORM_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Transform a minimum phase sequence into a frequency-warped sequence.
 *
 * The input is the \f$M_1\f$-th order minimum phase sequence:
 * \f[
 *   \begin{array}{cccc}
 *     c_{\alpha_1}(0), & c_{\alpha_1}(1), & \ldots, & c_{\alpha_1}(M_1),
 *   \end{array}
 * \f]
 * and the output is the \f$M_2\f$-th order frequency-warped sequence:
 * \f[
 *   \begin{array}{cccc}
 *     c_{\alpha_2}(0), & c_{\alpha_2}(1), & \ldots, & c_{\alpha_2}(M_2),
 *   \end{array}
 * \f]
 * The output sequence can be obtained by using the following recursion formula:
 * \f[
 *   c_{\alpha_2}^{(i)}(m) = \left\{\begin{array}{ll}
 *     c_{\alpha_1}(-i) + \alpha\,c_{\alpha_2}^{(i-1)}(0), & m=0 \\
 *     (1-\alpha^2)\,c_{\alpha_2}^{(i-1)}(0) +
 *       \alpha\,c_{\alpha_2}^{(i-1)}(1), & m=1 \\
 *     c_{\alpha_2}^{(i-1)}(m-1) + \alpha (c_{\alpha_2}^{(i-1)}(m) -
 *       c_{\alpha_2}^{(i)}(m-1)), & m=2,3,\ldots,M_2
 *   \end{array} \right. \\
 *   i = -M_1,\ldots,-1,0
 * \f]
 * where
 * \f[
 *   \alpha = (\alpha_2 - \alpha_1)\,/\,(1 - \alpha_1 \alpha_2).
 * \f]
 * The initial condition of the recursion is
 * \f$c_{\alpha_2}^{(-M_1-1)}(m) = 0\f$ for any \f$m\f$.
 *
 * The transformation is based on the cascade of all-pass networks. For more
 * detail, see [1]. Note that the above recursion can be represented as a linear
 * transformation, i.e., matrix multiplication.
 *
 * [1] A. Oppenheim and D. Johnson, "Discrete representation of signals,"
 *     Proc. of the IEEE, vol. 60, no. 6, pp. 681-691, 1972.
 *
 * [2] K. Tokuda, T. Kobayashi, T. Masuko, and S. Imai, "Mel-generalized
 *     cepstral representation of speech - A unified approach to speech spectral
 *     estimation," Proc. of ICSLP 1994, pp. 1043-1046, 1994.
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
   * @param[in] num_input_order Order of input, \f$M_1\f$.
   * @param[in] num_output_order Order of output, \f$M_2\f$.
   * @param[in] alpha Frequency warping factor, \f$\alpha\f$.
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
   * @return True if this obejct is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] minimum_phase_sequence \f$M_1\f$-th order input sequence.
   * @param[out] warped_sequence \f$M_2\f$-th order output sequence.
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
