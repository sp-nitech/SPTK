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
