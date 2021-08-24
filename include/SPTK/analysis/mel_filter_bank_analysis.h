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

#ifndef SPTK_ANALYSIS_MEL_FILTER_BANK_ANALYSIS_H_
#define SPTK_ANALYSIS_MEL_FILTER_BANK_ANALYSIS_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Perform mel-filter-bank analysis.
 *
 * The input is the half part of power spectrum:
 * @f[
 *   \begin{array}{cccc}
 *     |X(0)|^2, & |X(1)|^2, & \ldots, & |X(N/2)|^2,
 *   \end{array}
 * @f]
 * where @f$N@f$ is the FFT length. The outputs are the @f$C@f$-channel
 * mel-filter-bank outputs
 * @f[
 *   \begin{array}{cccc}
 *     F(1), & F(2), & \ldots, & F(C)
 *   \end{array}
 * @f]
 * and the log-signal energy @f$E@f$.
 *
 * The implementation is based on HTK. The only difference from the
 * implementation is the constant of mel-scale formula:
 * @f[
 *   m = 1127.01048 \log \left( 1 + \frac{f}{700} \right),
 * @f]
 * where HTK use @f$1127@f$ instead of @f$1127.01048@f$.
 *
 * [1] S. Young et al., &quot;The HTK book,&quot; Cambridge University
 *     Engineering Department, 2006.
 */
class MelFilterBankAnalysis {
 public:
  /**
   * @param[in] fft_length Number of FFT bins, @f$N@f$.
   * @param[in] num_channel Number of channels, @f$C@f$.
   * @param[in] sampling_rate Sampling rate in Hz.
   * @param[in] lowest_frequency Lowest frequency in Hz.
   * @param[in] highest_frequency Highest frequency in Hz.
   * @param[in] floor Floor value of raw filter-bank output.
   * @param[in] use_power If true, use power spectrum instead of amplitude one.
   */
  MelFilterBankAnalysis(int fft_length, int num_channel, double sampling_rate,
                        double lowest_frequency, double highest_frequency,
                        double floor, bool use_power);

  virtual ~MelFilterBankAnalysis() {
  }

  /**
   * @return FFT size.
   */
  int GetFftLength() const {
    return fft_length_;
  }

  /**
   * @return Number of channels.
   */
  int GetNumChannel() const {
    return num_channel_;
  }

  /**
   * @return Floor value.
   */
  double GetFloor() const {
    return floor_;
  }

  /**
   * @return Whether to use power spectrum.
   */
  bool IsPowerUsed() const {
    return use_power_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] power_spectrum @f$(N/2+1)@f$-length power spectrum.
   * @param[out] filter_bank_output @f$C@f$-channel filter-bank outputs.
   * @param[out] energy Signal energy @f$E@f$ (optional).
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& power_spectrum,
           std::vector<double>* filter_bank_output, double* energy) const;

 private:
  const int fft_length_;
  const int num_channel_;
  const double floor_;
  const bool use_power_;

  bool is_valid_;

  int lower_bin_index_;
  int upper_bin_index_;
  std::vector<int> channel_indices_;
  std::vector<double> channel_weights_;

  DISALLOW_COPY_AND_ASSIGN(MelFilterBankAnalysis);
};

}  // namespace sptk

#endif  // SPTK_ANALYSIS_MEL_FILTER_BANK_ANALYSIS_H_
