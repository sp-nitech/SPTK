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

#ifndef SPTK_CONVERSION_GENERALIZED_CEPSTRUM_INVERSE_GAIN_NORMALIZATION_H_
#define SPTK_CONVERSION_GENERALIZED_CEPSTRUM_INVERSE_GAIN_NORMALIZATION_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Convert normalized generalized cepstral coefficients to generalized
 * cepstral coefficients.
 *
 * The input is the @f$M@f$-th order normalized generalized cepstral
 * coefficients and the gain @f$K@f$:
 * @f[
 *   \begin{array}{cccc}
 *     K, & c'_\gamma(1), & \ldots, & c'_\gamma(M),
 *   \end{array}
 * @f]
 * and the output is the @f$M@f$-th order generalized cepstral coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     c_\gamma(0), & c_\gamma(1), & \ldots, & c_\gamma(M).
 *   \end{array}
 * @f]
 * The zeroth generalized cepstral coefficient is given by
 * @f[
 *   c_\gamma(0) = \left\{ \begin{array}{ll}
 *     (K^\gamma - 1) / \gamma, \quad & 0 < |\gamma| \le 1 \\
 *     \log \, K. & \gamma = 0
 *   \end{array} \right.
 * @f]
 * The other generalized cepstral coefficients are obtained as follows:
 * @f[
 *   c_\gamma(m) = (1 + \gamma \, c_\gamma(0)) c'_\gamma(m).
 * @f]
 *
 * [1] T. Kobayashi and S. Imai, &quot;Spectral analysis using generalized
 *     cepstrum,&quot; IEEE Transactions on Acoustics, Speech, and Signal
 *     Processing, vol. 32, no. 5, pp. 1087-1089, 1984.
 */
class GeneralizedCepstrumInverseGainNormalization {
 public:
  /**
   * @param[in] num_order Order of coefficients, @f$M@f$.
   * @param[in] gamma Exponent parameter, @f$\gamma@f$.
   */
  GeneralizedCepstrumInverseGainNormalization(int num_order, double gamma);

  virtual ~GeneralizedCepstrumInverseGainNormalization() {
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
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] normalized_generalized_cepstrum @f$M@f$-th order normalized
   *            cepstral coefficients.
   * @param[out] generalized_cepstrum @f$M@f$-th order cepstral coefficients.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& normalized_generalized_cepstrum,
           std::vector<double>* generalized_cepstrum) const;

  /**
   * @param[in,out] input_and_output @f$M@f$-th order coefficients.
   * @return True on success, false on failure.
   */
  bool Run(std::vector<double>* input_and_output) const;

 private:
  const int num_order_;
  const double gamma_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(GeneralizedCepstrumInverseGainNormalization);
};

}  // namespace sptk

#endif  // SPTK_CONVERSION_GENERALIZED_CEPSTRUM_INVERSE_GAIN_NORMALIZATION_H_
