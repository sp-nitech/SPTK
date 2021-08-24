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

#ifndef SPTK_CONVERSION_MEL_CEPSTRUM_TO_MLSA_DIGITAL_FILTER_COEFFICIENTS_H_
#define SPTK_CONVERSION_MEL_CEPSTRUM_TO_MLSA_DIGITAL_FILTER_COEFFICIENTS_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Convert mel-cepstral coefficients to MLSA digital filter coefficients.
 *
 * The input is the @f$M@f$-th order mel-cepstral coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     \tilde{c}(0), & \tilde{c}(1), & \ldots, & \tilde{c}(M),
 *   \end{array}
 * @f]
 * and the output is the @f$M@f$-th order MLSA digital filter coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     b(0), & b(1), & \ldots, & b(M).
 *   \end{array}
 * @f]
 * The MLSA digital filter coefficients can be obtained by the linear
 * transformation of the mel-cepstral coefficients:
 * @f[
 *   \boldsymbol{b} = \boldsymbol{A}^{-1}\tilde{\boldsymbol{c}},
 * @f]
 * where
 * @f{eqnarray}{
 *   \boldsymbol{A}^{-1} &=& \left[ \begin{array}{ccccc}
 *     1      & -\alpha & (-\alpha)^2 & \cdots & (-\alpha)^M  \\
 *     0      & 1       & -\alpha     & \ddots & \vdots       \\
 *     0      & 0       & 1           & \ddots & (-\alpha)^2  \\
 *     \vdots & \vdots  & \ddots      & \ddots & -\alpha      \\
 *     0      & 0       & \cdots      & 0      & 1            \\
 *   \end{array} \right], \\
 *   \boldsymbol{b} &=& \left[ \begin{array}{cccc}
 *      b(0) & b(1) & \cdots & b(M)
 *   \end{array} \right]^{\mathsf{T}}, \\
 *   \tilde{\boldsymbol{c}} &=& \left[ \begin{array}{cccc}
 *     \tilde{c}(0) & \tilde{c}(1) & \cdots & \tilde{c}(M)
 *   \end{array} \right]^{\mathsf{T}}.
 * @f}
 * The conversion is implemented with low computational complexity in a
 * recursive manner as follows:
 * @f[
 *   b(m) = \left\{ \begin{array}{ll}
 *     \tilde{c}(m), & m = M \\
 *     \tilde{c}(m) - \alpha b(m + 1). & 0 \le m < M
 *   \end{array} \right.
 * @f]
 *
 * [1] K. Tokuda, T. Kobayashi, T. Chiba, and S. Imai, &quot;Spectral estimation
 *     of speech by mel-generalized cepstral analysis,&quot; Electronics and
 *     Communications in Japan, part 3, vol. 76, no. 2, pp. 30-43, 1993.
 */
class MelCepstrumToMlsaDigitalFilterCoefficients {
 public:
  /**
   * @param[in] num_order Order of coefficients, @f$M@f$.
   * @param[in] alpha Frequency warping factor, @f$\alpha@f$.
   */
  MelCepstrumToMlsaDigitalFilterCoefficients(int num_order, double alpha);

  virtual ~MelCepstrumToMlsaDigitalFilterCoefficients() {
  }

  /**
   * @return Order of coefficients.
   */
  int GetNumOrder() const {
    return num_order_;
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
   * @param[in] mel_cepstrum @f$M@f$-th order mel-cepstral coefficients.
   * @param[out] mlsa_digital_filter_coefficients @f$M@f$-th order MLSA digital
   *             filter coefficients.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& mel_cepstrum,
           std::vector<double>* mlsa_digital_filter_coefficients) const;

  /**
   * @param[in,out] input_and_output @f$M@f$-th order coefficients.
   * @return True on success, false on failure.
   */
  bool Run(std::vector<double>* input_and_output) const;

 private:
  const int num_order_;
  const double alpha_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(MelCepstrumToMlsaDigitalFilterCoefficients);
};

}  // namespace sptk

#endif  // SPTK_CONVERSION_MEL_CEPSTRUM_TO_MLSA_DIGITAL_FILTER_COEFFICIENTS_H_
