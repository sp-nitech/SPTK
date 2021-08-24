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

#ifndef SPTK_ANALYSIS_FAST_FOURIER_TRANSFORM_CEPSTRAL_ANALYSIS_H_
#define SPTK_ANALYSIS_FAST_FOURIER_TRANSFORM_CEPSTRAL_ANALYSIS_H_

#include <vector>  // std::vector

#include "SPTK/math/real_valued_fast_fourier_transform.h"
#include "SPTK/math/real_valued_inverse_fast_fourier_transform.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Calculate cepstrum from power spectrum.
 *
 * The input is the half of power spectrum:
 * @f[
 *   \begin{array}{cccc}
 *     |X(0)|^2, & |X(1)|^2, & \ldots, & |X(N/2)|^2,
 *   \end{array}
 * @f]
 * where @f$N@f$ is the FFT length. The output is the @f$M@f$-th order cepstral
 * coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     c(0), & c(1), & \ldots, & c(M).
 *   \end{array}
 * @f]
 * The cepstral coefficients is obtained by applying the logarithmic fucntion
 * and the inverse FFT to the input power spectrum. However, the envelope
 * derived from the cepstral coefficients is not robust to the change of the
 * fine structure. To solve the problem, the improved cepstral analysis method
 * has been proposed. The algorithm attempts to estimate the true envelope in an
 * iterative manner. Only a few iteration, e.g., @f$J=3@f$, is sufficient to
 * estimate a good envelope. To speed up the convergence, the acceleration
 * factor, @f$\alpha@f$, is introduced where @f$\alpha=0@f$ means no
 * acceleration. For the detail, see [1].
 *
 * [1] S. Imai and Y. Abe, &quot;Spectral envelope extraction by improved
 *     cepstral method,&quot; Journal of IEICE, vol. J62-A, no. 4, pp. 217-223,
 *     1979 (Japanese).
 */
class FastFourierTransformCepstralAnalysis {
 public:
  /**
   * Buffer for FastFourierTransformCepstralAnalysis class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    std::vector<double> real_part_;
    std::vector<double> imag_part_;

    RealValuedFastFourierTransform::Buffer buffer_for_fast_fourier_transform_;
    RealValuedInverseFastFourierTransform::Buffer
        buffer_for_inverse_fast_fourier_transform_;

    friend class FastFourierTransformCepstralAnalysis;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] fft_length Number of FFT bins, @f$N@f$.
   * @param[in] num_order Order of cepstral coefficients, @f$M@f$.
   * @param[in] num_iteration Number of iterations of the improved cepstral
   *            analysis method, @f$J@f$.
   * @param[in] acceleration_factor Acceleration factor, @f$\alpha@f$.
   */
  FastFourierTransformCepstralAnalysis(int fft_length, int num_order,
                                       int num_iteration,
                                       double acceleration_factor);

  virtual ~FastFourierTransformCepstralAnalysis() {
  }

  /**
   * @return FFT length.
   */
  int GetFftLength() const {
    return fast_fourier_transform_.GetFftLength();
  }

  /**
   * @return Order of coefficients.
   */
  int GetNumOrder() const {
    return num_order_;
  }

  /**
   * @return Number of iterations.
   */
  int GetNumIteration() const {
    return num_iteration_;
  }

  /**
   * @return Accelaration factor.
   */
  double GetAccelarationFactor() const {
    return acceleration_factor_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] power_spectrum @f$(N/2+1)@f$-length power spectrum.
   * @param[out] cepstrum @f$M@f$-th order cepstral coefficients.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& power_spectrum,
           std::vector<double>* cepstrum,
           FastFourierTransformCepstralAnalysis::Buffer* buffer) const;

 private:
  const int num_order_;
  const int num_iteration_;
  const double acceleration_factor_;

  const RealValuedFastFourierTransform fast_fourier_transform_;
  const RealValuedInverseFastFourierTransform inverse_fast_fourier_transform_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(FastFourierTransformCepstralAnalysis);
};

}  // namespace sptk

#endif  // SPTK_ANALYSIS_FAST_FOURIER_TRANSFORM_CEPSTRAL_ANALYSIS_H_
