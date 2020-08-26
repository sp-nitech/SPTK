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

#ifndef SPTK_CONVERTER_MLSA_DIGITAL_FILTER_COEFFICIENTS_TO_MEL_CEPSTRUM_H_
#define SPTK_CONVERTER_MLSA_DIGITAL_FILTER_COEFFICIENTS_TO_MEL_CEPSTRUM_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Transform MLSA digital filter coefficients to mel-cepstral coefficients.
 *
 * The input is the \f$M\f$-th order MLSA digital filter coefficients:
 * \f[
 *   \begin{array}{cccc}
 *     b(0), & b(1), & \ldots, & b(M),
 *   \end{array}
 * \f]
 * and the output is the \f$M\f$-th order mel-cepstral coefficients:
 * \f[
 *   \begin{array}{cccc}
 *     \tilde{c}(0), & \tilde{c}(1), & \ldots, & \tilde{c}(M).
 *   \end{array}
 * \f]
 * The mel-cepstral coefficients can be obtained by the linear transformation
 * of the MLSA digital filter coefficients:
 * \f[
 *   \tilde{\boldsymbol{c}} = \boldsymbol{A}\boldsymbol{b},
 * \f]
 * where
 * \f{eqnarray}{
 *   \boldsymbol{A} &=& \left[ \begin{array}{ccccc}
 *     1      & \alpha & 0      & \cdots & 0      \\
 *     0      & 1      & \alpha & \ddots & \vdots \\
 *     0      & 0      & 1      & \ddots & 0      \\
 *     \vdots & \vdots & \ddots & \ddots & \alpha \\
 *     0      & 0      & \cdots & 0      & 1      \\
 *   \end{array} \right], \\
 *   \tilde{\boldsymbol{c}} &=& \left[ \begin{array}{cccc}
 *     \tilde{c}(0) & \tilde{c}(1) & \cdots & \tilde{c}(M)
 *   \end{array} \right]^{\mathsf{T}}, \\
 *   \boldsymbol{b} &=& \left[ \begin{array}{cccc}
 *      b(0) & b(1) & \cdots & b(M)
 *   \end{array} \right]^{\mathsf{T}}.
 * \f}
 * The transformation is implemented with low computational complexity as
 * follows:
 * \f[
 *   \tilde{c}(m) = \left\{ \begin{array}{ll}
 *     b(m), & m = M \\
 *     b(m) + \alpha b(m + 1). & 0 \le m < M
 *   \end{array} \right.
 * \f]
 *
 * [1] K. Tokuda, T. Kobayashi, T. Chiba, and S. Imai, &quot;Spectral estimation
 *     of speech by mel-generalized cepstral analysis,&quot; Electronics and
 *     Communications in Japan, part 3, vol. 76, no. 2, pp. 30-43, 1993.
 */
class MlsaDigitalFilterCoefficientsToMelCepstrum {
 public:
  /**
   * @param[in] num_order Order of coefficients, \f$M\f$.
   * @param[in] alpha Frequency warping factor, \f$\alpha\f$.
   */
  MlsaDigitalFilterCoefficientsToMelCepstrum(int num_order, double alpha);

  virtual ~MlsaDigitalFilterCoefficientsToMelCepstrum() {
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
   * @param[in] mlsa_digital_filter_coefficients \f$M\f$-th order MLSA digital
   *            filter coefficients.
   * @param[out] mel_cepstrum \f$M\f$-th order mel-cepstral coefficients.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& mlsa_digital_filter_coefficients,
           std::vector<double>* mel_cepstrum) const;

 private:
  const int num_order_;
  const double alpha_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(MlsaDigitalFilterCoefficientsToMelCepstrum);
};

}  // namespace sptk

#endif  // SPTK_CONVERTER_MLSA_DIGITAL_FILTER_COEFFICIENTS_TO_MEL_CEPSTRUM_H_
