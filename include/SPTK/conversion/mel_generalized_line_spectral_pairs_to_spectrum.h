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

#ifndef SPTK_CONVERSION_MEL_GENERALIZED_LINE_SPECTRAL_PAIRS_TO_SPECTRUM_H_
#define SPTK_CONVERSION_MEL_GENERALIZED_LINE_SPECTRAL_PAIRS_TO_SPECTRUM_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Convert mel-LSP to spectrum.
 *
 * The input is the @f$M@f$-th order line spectral paris:
 * @f[
 *   \begin{array}{cccc}
 *     K, & \omega(1), & \ldots, & \omega(M),
 *   \end{array}
 * @f]
 * where @f$K@f$ is the gain. The output is the @f$(L/2+1)@f$-length log
 * amplitude spectrum:
 * @f[
 *   \begin{array}{cccc}
 *     \log|H(0)|, & \log|H(1)|, & \ldots, & \log|H(L/2)|,
 *   \end{array}
 * @f]
 * where @f$L@f$ is the FFT length.
 *
 * The transfer function of the all-pole filter is given by
 * @f[
 *   H(z) = K \cdot A^{1/\gamma}(z).
 * @f]
 * Thus,
 * @f[
 *   \log |H(z)| = \log K + \frac{1}{2\gamma} \log |A(z)|^2.
 * @f]
 * If @f$M@f$ is even,
 * @f{eqnarray}{
 *   |A(\omega)|^2 &=& 2^{M} \left\{
 *      \cos^2 \left( \frac{\tilde{\omega}}{2} \right)
 *      \prod_{m=1,3,\ldots}^{M} (\cos\tilde{\omega}-\cos\omega(m))^2 \right. \\
 *      && \left. + \sin^2 \left( \frac{\tilde{\omega}}{2} \right)
 *      \prod_{m=2,4,\ldots}^{M} (\cos\tilde{\omega}-\cos\omega(m))^2
 *   \right\},
 * @f}
 * else
 * @f{eqnarray}{
 *   |A(\omega)|^2 &=& 2^{M-1} \left\{
 *      \prod_{m=1,3,\ldots}^{M} (\cos\tilde{\omega}-\cos\omega(m))^2 \right. \\
 *      && \left. + \sin^2 \tilde{\omega}
 *      \prod_{m=2,4,\ldots}^{M} (\cos\tilde{\omega}-\cos\omega(m))^2
 *   \right\},
 * @f}
 * where @f$\tilde{\omega}@f$ is the angular frequency warped by the first-order
 * all pass filter:
 * @f[
 *   \tilde{\omega} = \omega + 2\tan^{-1}
 *     \left( \frac{\alpha\sin\omega}{1 - \alpha\cos\omega} \right).
 * @f]
 *
 * [1] A. V. Oppenheim and D. H. Johnson, &quot;Discrete representation of
 *     signals,&quot; Proc. of the IEEE, vol. 60, no. 6, pp. 681-691, 1972.
 *
 * [2] N. Sugamura and F. Itakura, &quot;Speech data compression by LSP speech
 *     analysis-synthesis technique,&quot; Journal of IEICE, vol. J64-A, no. 8,
 *     pp. 599-606, 1981.
 */
class MelGeneralizedLineSpectralPairsToSpectrum {
 public:
  /**
   * @param[in] num_order Order of line spectral pairs, @f$M@f$.
   * @param[in] alpha Alpha, @f$\alpha@f$.
   * @param[in] gamma Gamma, @f$\gamma@f$.
   * @param[in] fft_length FFT length, @f$L@f$.
   */
  MelGeneralizedLineSpectralPairsToSpectrum(int num_order, double alpha,
                                            double gamma, int fft_length);

  virtual ~MelGeneralizedLineSpectralPairsToSpectrum() {
  }

  /**
   * @return Order of coefficients.
   */
  int GetNumOrder() const {
    return num_order_;
  }

  /**
   * @return Alpha.
   */
  double GetAlpha() const {
    return alpha_;
  }

  /**
   * @return Gamma.
   */
  double GetGamma() const {
    return gamma_;
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
   * @param[in] line_spectral_pairs @f$M@f$-th order line spectral pairs. The
   *            first element is linear gain and the other elements are in
   *            normalized frequency @f$(0, \pi)@f$.
   * @param[out] spectrum @f$(L/2+1)@f$-length log amplitude spectrum.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& line_spectral_pairs,
           std::vector<double>* spectrum) const;

 private:
  const int num_order_;
  const double alpha_;
  const double gamma_;
  const int fft_length_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(MelGeneralizedLineSpectralPairsToSpectrum);
};

}  // namespace sptk

#endif  // SPTK_CONVERSION_MEL_GENERALIZED_LINE_SPECTRAL_PAIRS_TO_SPECTRUM_H_
