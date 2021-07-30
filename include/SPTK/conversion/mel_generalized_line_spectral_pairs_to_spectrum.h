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
