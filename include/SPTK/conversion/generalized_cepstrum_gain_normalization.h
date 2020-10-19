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

#ifndef SPTK_CONVERSION_GENERALIZED_CEPSTRUM_GAIN_NORMALIZATION_H_
#define SPTK_CONVERSION_GENERALIZED_CEPSTRUM_GAIN_NORMALIZATION_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Transform generalized cepstral coefficients to normalized generalized
 * cepstral coefficients.
 *
 * The input is the \f$M\f$-th order generalized cepstral coefficients:
 * \f[
 *   \begin{array}{cccc}
 *     c_\gamma(0), & c_\gamma(1), & \ldots, & c_\gamma(M),
 *   \end{array}
 * \f]
 * and the output is the \f$M\f$-th order normalized generalized cepstral
 * coefficients:
 * \f[
 *   \begin{array}{cccc}
 *     K, & c'_\gamma(1), & \ldots, & c'_\gamma(M),
 *   \end{array}
 * \f]
 * where \f$K\f$ is the gain represented as
 * \f[
 *   K = \left\{ \begin{array}{ll}
 *     (1 + \gamma\,c_\gamma(0))^{1/\gamma}, \quad & 0 < |\gamma| \le 1 \\
 *     \exp \, c_\gamma(0). & \gamma = 0
 *   \end{array} \right.
 * \f]
 * The normalized generalized cepstral coefficients are obtained as follows:
 * \f[
 *   c'_\gamma(m) = \frac{c_\gamma(m)}{1 + \gamma \, c_\gamma(0)}.
 * \f]
 *
 * [1] T. Kobayashi and S. Imai, &quot;Spectral analysis using generalized
 *     cepstrum,&quot; IEEE Transactions on Acoustics, Speech, and Signal
 *     Processing, vol. 32, no. 5, pp. 1087-1089, 1984.
 */
class GeneralizedCepstrumGainNormalization {
 public:
  /**
   * @param[in] num_order Order of coefficients, \f$M\f$.
   * @param[in] gamma Exponent parameter, \f$\gamma\f$.
   */
  GeneralizedCepstrumGainNormalization(int num_order, double gamma);

  virtual ~GeneralizedCepstrumGainNormalization() {
  }

  /**
   * @return Order of coefficients.
   */
  int GetNumOrder() const {
    return num_order_;
  }

  /**
   * @return Exponent parameter.
   */
  double GetGamma() const {
    return gamma_;
  }

  /**
   * @return True if this obejct is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] generalized_cepstrum \f$M\f$-th order cepstral coefficients.
   * @param[out] normalized_generalized_cepstrum \f$M\f$-th order normalized
   *             cepstral coefficients.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& generalized_cepstrum,
           std::vector<double>* normalized_generalized_cepstrum) const;

  /**
   * @param[in,out] input_and_output \f$M\f$-th order coefficients.
   * @return True on success, false on failure.
   */
  bool Run(std::vector<double>* input_and_output) const;

 private:
  const int num_order_;
  const double gamma_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(GeneralizedCepstrumGainNormalization);
};

}  // namespace sptk

#endif  // SPTK_CONVERSION_GENERALIZED_CEPSTRUM_GAIN_NORMALIZATION_H_
