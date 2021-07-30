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
//                1996-2021  Nagoya Institute of Technology          //
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
