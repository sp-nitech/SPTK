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
 * \f[
 *   \begin{array}{cccc}
 *     |X(0)|^2, & |X(1)|^2, & \ldots, & |X(N/2)|^2,
 *   \end{array}
 * \f]
 * where \f$N\f$ is the FFT length. The output is the \f$M\f$-th order cepstral
 * coefficients:
 * \f[
 *   \begin{array}{cccc}
 *     c(0), & c(1), & \ldots, & c(M).
 *   \end{array}
 * \f]
 * The cepstral coefficients is obtained by applying the logarithmic fucntion
 * and the inverse FFT to the input power spectrum. However, the envelope
 * derived from the cepstral coefficients is not robust to the change of the
 * fine structure. To solve the problem, the improved cepstral analysis method
 * has been proposed. The algorithm attempts to estimate the true envelope in an
 * iterative manner. Only a few iteration, e.g., \f$J=3\f$, is sufficient to
 * estimate a good envelope. To speed up the convergence, the acceleration
 * factor, \f$\alpha\f$, is introduced where \f$\alpha=0\f$ means no
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
   * @param[in] fft_length Number of FFT bins, \f$N\f$.
   * @param[in] num_order Order of cepstral coefficients, \f$M\f$.
   * @param[in] num_iteration Number of iterations of the improved cepstral
   *            analysis method, \f$J\f$.
   * @param[in] acceleration_factor Acceleration factor, \f$\alpha\f$.
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
   * @return True if this obejct is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] power_spectrum \f$(N/2+1)\f$-length power spectrum.
   * @param[out] cepstrum \f$M\f$-th order cepstral coefficients.
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
