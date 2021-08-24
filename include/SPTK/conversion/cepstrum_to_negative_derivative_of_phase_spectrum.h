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

#ifndef SPTK_CONVERSION_CEPSTRUM_TO_NEGATIVE_DERIVATIVE_OF_PHASE_SPECTRUM_H_
#define SPTK_CONVERSION_CEPSTRUM_TO_NEGATIVE_DERIVATIVE_OF_PHASE_SPECTRUM_H_

#include <vector>  // std::vector

#include "SPTK/math/real_valued_fast_fourier_transform.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Convert cepstral coefficients to negative derivative of phase spectrum.
 *
 * The input is the @f$M@f$-th order cepstral coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     c(0), & c(1), & \ldots, & c(M),
 *   \end{array}
 * @f]
 * and the output is the @f$(L/2+1)@f$-length NDPS:
 * @f[
 *   \begin{array}{cccc}
 *     n(0), & n(1), & \ldots, & n(L/2+1),
 *   \end{array}
 * @f]
 * where @f$L@f$ must be a power of two.
 *
 * The log spectrum can be represented as
 * @f[
 *   \log S(\omega) = \sum_{m=0}^{M} c(m) e^{-j\omega m}.
 * @f]
 * It can be decomposed into the real part and imaginary part:
 * @f[
 *   \log |S(\omega)| + j\arg S(\omega) = \sum_{m=0}^{M} c(m) e^{-j\omega m}.
 * @f]
 * By differentiating the equation with respect to @f$\omega@f$, we obtain
 * @f[
 *   \frac{\partial}{\partial \omega} \log |S(\omega)|
 *   +j \frac{\partial}{\partial \omega} \arg S(\omega) =
 *   -j \sum_{m=0}^{M} m \cdot c(m) e^{-j\omega m}.
 * @f]
 * From the imaginary part of the above equation, NDPS is obtained as
 * @f[
 *   -\frac{\partial}{\partial \omega} \arg S(\omega) =
 *     \sum_{m=0}^{M} m \cdot c(m) \cos(\omega m).
 * @f]
 * This is equivalent to the real part of the DFT of @f$m\,c(m)@f$:
 * @f[
 *   n(k) = \mathrm{Re} \left[
 *     \sum_{m=0}^{M} m \cdot c(m) e^{-j2\pi mk / L}
 *   \right].
 * @f]
 * Note that @f$c(0)@f$ is not used in the calculation.
 *
 * [1] B. Yegnanarayana, &quot;Pole-zero decomposition of speech spectra,&quot;
 *     Signal Processing, vol. 3, no. 1, pp. 5-17, 1981.
 */
class CepstrumToNegativeDerivativeOfPhaseSpectrum {
 public:
  /**
   * Buffer for CepstrumToNegativeDerivativeOfPhaseSpectrum class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    std::vector<double> fast_fourier_transform_real_part_;
    std::vector<double> fast_fourier_transform_imag_part_;
    RealValuedFastFourierTransform::Buffer fast_fourier_transform_buffer_;

    friend class CepstrumToNegativeDerivativeOfPhaseSpectrum;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_order Order of cepstrum, @f$M@f$.
   * @param[in] fft_length Length of NDPS, @f$L@f$.
   */
  CepstrumToNegativeDerivativeOfPhaseSpectrum(int num_order, int fft_length);

  virtual ~CepstrumToNegativeDerivativeOfPhaseSpectrum() {
  }

  /**
   * @return Order of cepstrum.
   */
  int GetNumOrder() const {
    return num_order_;
  }

  /**
   * @return FFT length.
   */
  int GetFftLength() const {
    return fast_fourier_transform_.GetFftLength();
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] cepstrum @f$M@f$-th order cepstrum.
   * @param[out] negative_derivative_of_phase_spectrum @f$(L/2+1)@f$-length
   *             NDPS.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& cepstrum,
           std::vector<double>* negative_derivative_of_phase_spectrum,
           CepstrumToNegativeDerivativeOfPhaseSpectrum::Buffer* buffer) const;

 private:
  const int num_order_;

  const RealValuedFastFourierTransform fast_fourier_transform_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(CepstrumToNegativeDerivativeOfPhaseSpectrum);
};

}  // namespace sptk

#endif  // SPTK_CONVERSION_CEPSTRUM_TO_NEGATIVE_DERIVATIVE_OF_PHASE_SPECTRUM_H_
