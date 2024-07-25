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

#ifndef SPTK_ANALYSIS_PERCEPTUAL_LINEAR_PREDICTIVE_COEFFICIENTS_ANALYSIS_H_
#define SPTK_ANALYSIS_PERCEPTUAL_LINEAR_PREDICTIVE_COEFFICIENTS_ANALYSIS_H_

#include <vector>  // std::vector

#include "SPTK/analysis/mel_filter_bank_analysis.h"
#include "SPTK/conversion/linear_predictive_coefficients_to_cepstrum.h"
#include "SPTK/math/inverse_fourier_transform.h"
#include "SPTK/math/levinson_durbin_recursion.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Perform perceptual linear predictive (PLP) coefficients analysis.
 *
 * The input is the half part of power spectrum:
 * @f[
 *   \begin{array}{cccc}
 *     |X(0)|^2, & |X(1)|^2, & \ldots, & |X(N/2)|^2,
 *   \end{array}
 * @f]
 * where @f$N@f$ is the FFT length. The outputs are the @f$M@f$-th order PLP
 * features with the zeroth cepstral parameter:
 * @f[
 *   \begin{array}{ccccc}
 *     c(0), & \bar{c}(1), & \bar{c}(2), & \ldots, & \bar{c}(M)
 *   \end{array}
 * @f]
 * and the log-signal energy @f$E@f$.
 *
 * [1] S. Young et al., &quot;The HTK book,&quot; Cambridge University
 *     Engineering Department, 2006.
 */
class PerceptualLinearPredictiveCoefficientsAnalysis {
 public:
  /**
   * Buffer for PerceptualLinearPredictiveCoefficientsAnalysis class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    std::vector<double> filter_bank_output_;
    std::vector<double> spectrum_;
    std::vector<double> cepstrum_;

    std::vector<double> real_part_input_;
    std::vector<double> real_part_output_;
    std::vector<double> imag_part_input_;
    std::vector<double> imag_part_output_;

    LevinsonDurbinRecursion::Buffer buffer_for_levinson_durbin_recursion_;

    friend class PerceptualLinearPredictiveCoefficientsAnalysis;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] fft_length Number of FFT bins, @f$N@f$.
   * @param[in] num_channel Number of channels, @f$C@f$.
   * @param[in] num_order Order of cepstral coefficients, @f$M@f$.
   * @param[in] liftering_coefficient A parameter of liftering, @f$L@f$.
   * @param[in] compression_factor Amplitude compression factor.
   * @param[in] sampling_rate Sampling rate in Hz.
   * @param[in] lowest_frequency Lowest frequency in Hz.
   * @param[in] highest_frequency Highest frequency in Hz.
   * @param[in] floor Floor value of raw filter-bank output.
   */
  PerceptualLinearPredictiveCoefficientsAnalysis(
      int fft_length, int num_channel, int num_order, int liftering_coefficient,
      double compression_factor, double sampling_rate, double lowest_frequency,
      double highest_frequency, double floor);

  virtual ~PerceptualLinearPredictiveCoefficientsAnalysis() {
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
    return levinson_durbin_recursion_.GetNumOrder();
  }

  /**
   * @return Liftering coefficient.
   */
  int GetLifteringCoefficient() const {
    return liftering_coefficient_;
  }

  /**
   * @return Compression factor.
   */
  double GetCompressionFactor() const {
    return compression_factor_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] power_spectrum @f$(N/2+1)@f$-length power spectrum.
   * @param[out] plp @f$M@f$-th order PLP features.
   * @param[out] energy Signal energy @f$E@f$ (optional).
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(
      const std::vector<double>& power_spectrum, std::vector<double>* plp,
      double* energy,
      PerceptualLinearPredictiveCoefficientsAnalysis::Buffer* buffer) const;

 private:
  const int liftering_coefficient_;
  const double compression_factor_;

  const MelFilterBankAnalysis mel_filter_bank_analysis_;
  const InverseFourierTransform inverse_fourier_transform_;
  const LevinsonDurbinRecursion levinson_durbin_recursion_;
  const LinearPredictiveCoefficientsToCepstrum
      linear_predictive_coefficients_to_cepstrum_;

  bool is_valid_;

  std::vector<double> equal_loudness_curve_;
  std::vector<double> cepstal_weights_;

  DISALLOW_COPY_AND_ASSIGN(PerceptualLinearPredictiveCoefficientsAnalysis);
};

}  // namespace sptk

#endif  // SPTK_ANALYSIS_PERCEPTUAL_LINEAR_PREDICTIVE_COEFFICIENTS_ANALYSIS_H_
