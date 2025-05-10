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

#ifndef SPTK_ANALYSIS_GOERTZEL_ANALYSIS_H_
#define SPTK_ANALYSIS_GOERTZEL_ANALYSIS_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Calculate DFT values at specified frequencies using the Goertzel algorithm.
 *
 * The input is the @f$L@f$-length waveform signals:
 * @f[
 *   \begin{array}{cccc}
 *     x(0), & x(1), & \ldots, & x(L-1).
 *   \end{array}
 * @f]
 * The outputs are the real and imaginary parts of the DFT values:
 * @f[
 *   \begin{array}{cccc}
 *   \mathrm{Re}(X(0)), & \mathrm{Re}(X(1)), & \ldots, & \mathrm{Re}(X(K-1)), \\
 *   \mathrm{Im}(X(0)), & \mathrm{Im}(X(1)), & \ldots, & \mathrm{Im}(X(K-1)),
 *   \end{array}
 * @f]
 * where @f$K@f$ is the number of frequencies to be analyzed.
 */
class GoertzelAnalysis {
 public:
  /**
   * @param[in] sampling_rate Sampling rate in Hz.
   * @param[in] frequencies @f$K@f$ frequencies in Hz to be analyzed.
   * @param[in] fft_length Number of points assumed in DFT. This is used to
   *            determine frequency bin resolution.
   */
  GoertzelAnalysis(double sampling_rate, const std::vector<double>& frequencies,
                   int fft_length);

  virtual ~GoertzelAnalysis() {
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] signals @f$L@f$-length waveform signals.
   * @param[out] real_part_output @f$K@f$ real parts.
   * @param[out] imag_part_output @f$K@f$ imaginary parts.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& signals,
           std::vector<double>* real_part_output,
           std::vector<double>* imag_part_output) const;

 private:
  bool is_valid_;

  std::vector<double> sine_table_;
  std::vector<double> cosine_table_;

  DISALLOW_COPY_AND_ASSIGN(GoertzelAnalysis);
};

}  // namespace sptk

#endif  // SPTK_ANALYSIS_GOERTZEL_ANALYSIS_H_
