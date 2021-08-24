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

#ifndef SPTK_CONVERSION_WAVEFORM_TO_AUTOCORRELATION_H_
#define SPTK_CONVERSION_WAVEFORM_TO_AUTOCORRELATION_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Calculate autocorrelation.
 *
 * The input is the framed waveform signal:
 * @f[
 *   \begin{array}{cccc}
 *     x(0), & x(1), & \ldots, & x(L-1),
 *   \end{array}
 * @f]
 * where @f$L@f$ is the frame length. The output is the @f$M@f$-th order
 * autocorrelation coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     r(0), & r(1), & \ldots, & r(M).
 *   \end{array}
 * @f]
 * The autocorrelation is given by
 * @f[
 *   r(m) = \sum_{l=0}^{L-1-m} x(l)x(l+m),
 * @f]
 * where @f$m@f$ is the lag.
 */
class WaveformToAutocorrelation {
 public:
  /**
   * @param[in] frame_length Frame length, @f$L@f$.
   * @param[in] num_order Order of autocorrelation, @f$M@f$.
   */
  WaveformToAutocorrelation(int frame_length, int num_order);

  virtual ~WaveformToAutocorrelation() {
  }

  /**
   * @return Frame length.
   */
  int GetFrameLength() const {
    return frame_length_;
  }

  /**
   * @return Order of autocorrelation.
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
   * @param[in] waveform @f$L@f$-length framed waveform.
   * @param[out] autocorrelation @f$M@f$-th order autocorrelation coefficients.
   */
  bool Run(const std::vector<double>& waveform,
           std::vector<double>* autocorrelation) const;

 private:
  const int frame_length_;
  const int num_order_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(WaveformToAutocorrelation);
};

}  // namespace sptk

#endif  // SPTK_CONVERSION_WAVEFORM_TO_AUTOCORRELATION_H_
