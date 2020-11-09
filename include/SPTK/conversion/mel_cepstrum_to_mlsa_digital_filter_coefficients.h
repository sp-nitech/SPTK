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
   * @return True if this obejct is valid.
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
