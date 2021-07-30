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
