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

#ifndef SPTK_CONVERSION_NEGATIVE_DERIVATIVE_OF_PHASE_SPECTRUM_TO_CEPSTRUM_H_
#define SPTK_CONVERSION_NEGATIVE_DERIVATIVE_OF_PHASE_SPECTRUM_TO_CEPSTRUM_H_

#include <vector>  // std::vector

#include "SPTK/math/real_valued_fast_fourier_transform.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Convert negative derivative of phase spectrum to cepstral coefficients.
 *
 * The input is the @f$(L/2+1)@f$-length NDPS:
 * @f[
 *   \begin{array}{cccc}
 *     n(0), & n(1), & \ldots, & n(L/2+1),
 *   \end{array}
 * @f]
 * and the output is the @f$M@f$-th order cepstral coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     0, & c(1), & \ldots, & c(M).
 *   \end{array}
 * @f]
 * The cepstral coefficients are obtained by using the inverse DFT:
 * @f[
 *   c(m) = \frac{1}{mL} \sum_{k=0}^{L-1} n(k) e^{j2\pi mk / L}.
 * @f]
 *
 * [1] B. Yegnanarayana, &quot;Pole-zero decomposition of speech spectra,&quot;
 *     Signal Processing, vol. 3, no. 1, pp. 5-17, 1981.
 */
class NegativeDerivativeOfPhaseSpectrumToCepstrum {
 public:
  /**
   * Buffer for NegativeDerivativeOfPhaseSpectrumToCepstrum class.
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

    friend class NegativeDerivativeOfPhaseSpectrumToCepstrum;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] fft_length Length of NDPS, @f$L@f$.
   * @param[in] num_order Order of cepstrum, @f$M@f$.
   */
  NegativeDerivativeOfPhaseSpectrumToCepstrum(int fft_length, int num_order);

  virtual ~NegativeDerivativeOfPhaseSpectrumToCepstrum() {
  }

  /**
   * @return FFT length.
   */
  int GetFftLength() const {
    return fast_fourier_transform_.GetFftLength();
  }

  /**
   * @return Order of cepstrum.
   */
  int GetNumOrder() const {
    return num_order_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] negative_derivative_of_phase_spectrum @f$(L/2+1)@f$-length NDPS.
   * @param[out] cepstrum @f$M@f$-th order cepstrum.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& negative_derivative_of_phase_spectrum,
           std::vector<double>* cepstrum,
           NegativeDerivativeOfPhaseSpectrumToCepstrum::Buffer* buffer) const;

 private:
  const int num_order_;

  const RealValuedFastFourierTransform fast_fourier_transform_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(NegativeDerivativeOfPhaseSpectrumToCepstrum);
};

}  // namespace sptk

#endif  // SPTK_CONVERSION_NEGATIVE_DERIVATIVE_OF_PHASE_SPECTRUM_TO_CEPSTRUM_H_
