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

#ifndef SPTK_UTILS_MEL_CEPSTRUM_POSTFILTERING_H_
#define SPTK_UTILS_MEL_CEPSTRUM_POSTFILTERING_H_

#include <vector>  // std::vector

#include "SPTK/conversion/cepstrum_to_autocorrelation.h"
#include "SPTK/conversion/mel_cepstrum_to_mlsa_digital_filter_coefficients.h"
#include "SPTK/conversion/mlsa_digital_filter_coefficients_to_mel_cepstrum.h"
#include "SPTK/math/frequency_transform.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Emphasize formant in mel-cepstum domain.
 *
 * The input is the @f$M@f$-th order mel-cepstral coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     \tilde{c}(0), & \tilde{c}(1), & \ldots, & \tilde{c}(M),
 *   \end{array}
 * @f]
 * and the output is the @f$M@f$-th order postfiltered ones:
 * @f[
 *   \begin{array}{cccc}
 *     \tilde{c}_\beta(0), & \tilde{c}_\beta(1), & \ldots, & \tilde{c}_\beta(M),
 *   \end{array}
 * @f]
 * where @f$\beta@f$ is the intensity of postfiltering.
 *
 * [1] T. Yoshimura, K. Tokuda, T. Masuko, and T. Kobayashi, &quot;Incorporating
 *     a mixed excitation model and postfilter into HMM-based text-to-speech
 *     synthesis,&quot; Systems and Computers in Japan, vol. 36, no. 12,
 *     pp. 43-50, 2005.
 */
class MelCepstrumPostfiltering {
 public:
  /**
   * Buffer for MelCepstrumPostfiltering class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    std::vector<double> cepstrum_;
    std::vector<double> autocorrelation_;
    std::vector<double> mel_cepstrum_;
    std::vector<double> mlsa_digital_filter_coefficients_;

    FrequencyTransform::Buffer buffer_for_frequency_transform_;
    CepstrumToAutocorrelation::Buffer buffer_for_cepstrum_to_autocorrelation_;

    friend class MelCepstrumPostfiltering;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_order Order of mel-cepstral coefficients, @f$M@f$.
   * @param[in] num_cepstrum_order Order of cepstral coefficients, @f$N@f$,
   *            to approximate mel-cepstrum.
   * @param[in] impulse_response_length Length of impulse response, @f$L@f$.
   * @param[in] onset_index This is typically set to two, i.e., 0th and 1st
   *            mel-cepstral coefficients are not emphasized.
   * @param[in] alpha All-pass constant, @f$\alpha@f$.
   * @param[in] beta Intensity of postfiltering, @f$\beta@f$.
   */
  MelCepstrumPostfiltering(int num_order, int num_cepstrum_order,
                           int impulse_response_length, int onset_index,
                           double alpha, double beta);

  virtual ~MelCepstrumPostfiltering() {
  }

  /**
   * @return Order of mel-cepstral coefficients.
   */
  int GetNumOrder() const {
    return frequency_transform_.GetNumInputOrder();
  }

  /**
   * @return Order of cepstral coefficients.
   */
  int GetCepstrumOrder() const {
    return frequency_transform_.GetNumOutputOrder();
  }

  /**
   * @return Impulse response length.
   */
  int GetImpulseResponseLength() const {
    return cepstrum_to_autocorrelation_.GetFftLength();
  }

  /**
   * @return Onset index.
   */
  int GetOnsetIndex() const {
    return onset_index_;
  }

  /**
   * @return Alpha.
   */
  double GetAlpha() const {
    return -frequency_transform_.GetAlpha();
  }

  /**
   * @return Beta.
   */
  double GetBeta() const {
    return beta_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] mel_cepstrum @f$M@f$-th order mel-cepstral coefficients.
   * @param[out] postfiltered_mel_cepstrum @f$M@f$-th order postfiltered ones.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& mel_cepstrum,
           std::vector<double>* postfiltered_mel_cepstrum,
           MelCepstrumPostfiltering::Buffer* buffer) const;

  /**
   * @param[in,out] input_and_output @f$M@f$-th order mel-cepstral coefficients.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(std::vector<double>* input_and_output,
           MelCepstrumPostfiltering::Buffer* buffer) const;

 private:
  const int onset_index_;
  const double beta_;

  const FrequencyTransform frequency_transform_;
  const CepstrumToAutocorrelation cepstrum_to_autocorrelation_;
  const MelCepstrumToMlsaDigitalFilterCoefficients
      mel_cepstrum_to_mlsa_digital_filter_coefficients_;
  const MlsaDigitalFilterCoefficientsToMelCepstrum
      mlsa_digital_filter_coefficients_to_mel_cepstrum_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(MelCepstrumPostfiltering);
};

}  // namespace sptk

#endif  // SPTK_UTILS_MEL_CEPSTRUM_POSTFILTERING_H_
