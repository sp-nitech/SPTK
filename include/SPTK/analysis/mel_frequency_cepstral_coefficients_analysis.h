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

#ifndef SPTK_ANALYSIS_MEL_FREQUENCY_CEPSTRAL_COEFFICIENTS_ANALYSIS_H_
#define SPTK_ANALYSIS_MEL_FREQUENCY_CEPSTRAL_COEFFICIENTS_ANALYSIS_H_

#include <vector>  // std::vector

#include "SPTK/analysis/mel_filter_bank_analysis.h"
#include "SPTK/math/discrete_cosine_transform.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Perform mel-frequency cepstral coefficients (MFCC) analysis.
 *
 * The input is the half part of power spectrum:
 * @f[
 *   \begin{array}{cccc}
 *     |X(0)|^2, & |X(1)|^2, & \ldots, & |X(N/2)|^2,
 *   \end{array}
 * @f]
 * where @f$N@f$ is the FFT length. The outputs are the @f$M@f$-th order MFCCs
 * with the zeroth cepstral parameter:
 * @f[
 *   \begin{array}{ccccc}
 *     c(0), & \bar{c}(1), & \bar{c}(2), & \ldots, & \bar{c}(M)
 *   \end{array}
 * @f]
 * and the log-signal energy @f$E@f$.
 *
 * The MFCCs are calculated from mel-filter-bank outputs @f$\{F(j)\}_{j=1}^C@f$
 * using the discrete cosine transform
 * @f[
 *   c(m) = \sqrt{\frac{2}{C}} \sum_{j=1}^C F(j)
 *          \cos \left( \frac{\pi m}{C} \left(j - \frac{1}{2}\right) \right),
 * @f]
 * and the liftering
 * @f[
 *   \bar{c}(m) = \left( 1 + \frac{L}{2} \sin \frac{\pi m}{L} \right) c(m),
 * @f]
 * where @f$L@f$ is the liftering parameter.
 *
 * [1] S. Young et al., &quot;The HTK book,&quot; Cambridge University
 *     Engineering Department, 2006.
 */
class MelFrequencyCepstralCoefficientsAnalysis {
 public:
  /**
   * Buffer for MelFrequencyCepstralCoefficientsAnalysis class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    std::vector<double> filter_bank_output_;
    std::vector<double> cepstrum_;
    std::vector<double> imag_part_input_;
    std::vector<double> imag_part_output_;

    DiscreteCosineTransform::Buffer buffer_for_discrete_cosine_transform_;

    friend class MelFrequencyCepstralCoefficientsAnalysis;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] fft_length Number of FFT bins, @f$N@f$.
   * @param[in] num_channel Number of channels, @f$C@f$.
   * @param[in] num_order Order of cepstral coefficients, @f$M@f$.
   * @param[in] liftering_coefficient A parameter of liftering, @f$L@f$.
   * @param[in] sampling_rate Sampling rate in Hz.
   * @param[in] lowest_frequency Lowest frequency in Hz.
   * @param[in] highest_frequency Highest frequency in Hz.
   * @param[in] floor Floor value of raw filter-bank output.
   */
  MelFrequencyCepstralCoefficientsAnalysis(
      int fft_length, int num_channel, int num_order, int liftering_coefficient,
      double sampling_rate, double lowest_frequency, double highest_frequency,
      double floor);

  virtual ~MelFrequencyCepstralCoefficientsAnalysis() {
  }

  /**
   * @return FFT size.
   */
  int GetFftLength() const {
    return mel_filter_bank_analysis_.GetFftLength();
  }

  /**
   * @return Number of channels.
   */
  int GetNumChannel() const {
    return mel_filter_bank_analysis_.GetNumChannel();
  }

  /**
   * @return Order of cepstral coefficients.
   */
  int GetNumOrder() const {
    return num_order_;
  }

  /**
   * @return Liftering coefficient.
   */
  int GetLifteringCoefficient() const {
    return liftering_coefficient_;
  }

  /**
   * @return True if this obejct is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] power_spectrum @f$(N/2+1)@f$-length power spectrum.
   * @param[out] mfcc @f$M@f$-th order MFCCs.
   * @param[out] energy Signal energy @f$E@f$ (optional).
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& power_spectrum, std::vector<double>* mfcc,
           double* energy,
           MelFrequencyCepstralCoefficientsAnalysis::Buffer* buffer) const;

 private:
  const int num_order_;
  const int liftering_coefficient_;

  const MelFilterBankAnalysis mel_filter_bank_analysis_;
  const DiscreteCosineTransform discrete_cosine_transform_;

  bool is_valid_;

  std::vector<double> cepstal_weights_;

  DISALLOW_COPY_AND_ASSIGN(MelFrequencyCepstralCoefficientsAnalysis);
};

}  // namespace sptk

#endif  // SPTK_ANALYSIS_MEL_FREQUENCY_CEPSTRAL_COEFFICIENTS_ANALYSIS_H_
