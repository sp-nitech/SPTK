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
   * @return True if this obejct is valid.
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
