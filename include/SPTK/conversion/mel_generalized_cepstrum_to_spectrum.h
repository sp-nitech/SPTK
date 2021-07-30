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

#ifndef SPTK_CONVERSION_MEL_GENERALIZED_CEPSTRUM_TO_SPECTRUM_H_
#define SPTK_CONVERSION_MEL_GENERALIZED_CEPSTRUM_TO_SPECTRUM_H_

#include <vector>  // std::vector

#include "SPTK/conversion/mel_generalized_cepstrum_to_mel_generalized_cepstrum.h"
#include "SPTK/math/real_valued_fast_fourier_transform.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Convert mel-generalized cepstrum to spectrum.
 *
 * This is a simple combination of
 * MelGeneralizedCepstrumToMelGeneralizedCepstrum and
 * RealValuedFastFourierTransform.
 */
class MelGeneralizedCepstrumToSpectrum {
 public:
  /**
   * Buffer for MelGeneralizedCepstrumToSpectrum class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    MelGeneralizedCepstrumToMelGeneralizedCepstrum::Buffer
        mel_generalized_cepstrum_transform_buffer_;
    RealValuedFastFourierTransform::Buffer fast_fourier_transform_buffer_;
    std::vector<double> cepstrum_;

    friend class MelGeneralizedCepstrumToSpectrum;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_order Order of coefficients, @f$M@f$.
   * @param[in] alpha All-pass constant, @f$\alpha@f$.
   * @param[in] gamma Exponent parameter, @f$\gamma@f$.
   * @param[in] is_normalized gain-normalized flag.
   * @param[in] is_multiplied gamma-multiplied flag.
   * @param[in] fft_length FFT length, @f$N@f$.
   */
  MelGeneralizedCepstrumToSpectrum(int num_order, double alpha, double gamma,
                                   bool is_normalized, bool is_multiplied,
                                   int fft_length);

  virtual ~MelGeneralizedCepstrumToSpectrum() {
  }

  /**
   * @return Order of coefficients.
   */
  int GetNumOrder() const {
    return mel_generalized_cepstrum_transform_.GetNumInputOrder();
  }

  /**
   * @return Alpha.
   */
  double GetAlpha() const {
    return mel_generalized_cepstrum_transform_.GetInputAlpha();
  }

  /**
   * @return Gamma.
   */
  double GetGamma() const {
    return mel_generalized_cepstrum_transform_.GetInputGamma();
  }

  /**
   * @return True if input is gain-normalized.
   */
  bool IsNormalized() const {
    return mel_generalized_cepstrum_transform_.IsNormalizedInput();
  }

  /**
   * @return True if input is multiplied by gamma.
   */
  bool IsMultiplied() const {
    return mel_generalized_cepstrum_transform_.IsMultipliedInput();
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
   * @param[in] mel_generalized_cepstrum @f$M@f$-th order mel-generalized
   *            cepstrum.
   * @param[out] amplitude_spectrum @f$N@f$-length amplitude spectrum.
   * @param[out] phase_spectrum @f$N@f$-length phase spectrum.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& mel_generalized_cepstrum,
           std::vector<double>* amplitude_spectrum,
           std::vector<double>* phase_spectrum,
           MelGeneralizedCepstrumToSpectrum::Buffer* buffer) const;

 private:
  const MelGeneralizedCepstrumToMelGeneralizedCepstrum
      mel_generalized_cepstrum_transform_;
  const RealValuedFastFourierTransform fast_fourier_transform_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(MelGeneralizedCepstrumToSpectrum);
};

}  // namespace sptk

#endif  // SPTK_CONVERSION_MEL_GENERALIZED_CEPSTRUM_TO_SPECTRUM_H_
