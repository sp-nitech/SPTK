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

#ifndef SPTK_COMPRESSION_DYNAMIC_RANGE_COMPRESSION_H_
#define SPTK_COMPRESSION_DYNAMIC_RANGE_COMPRESSION_H_

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Balances the range between the loudest and quietest signals.
 *
 * The input is a signal @f$x(t)@f$ in the @f$T@f$-length signals:
 * @f[
 *   \begin{array}{cccc}
 *     x(0), & x(1), & \ldots, & x(T-1),
 *   \end{array}
 * @f]
 * and the output is the processed corresponding signal @f$y(t)@f$ in
 * @f[
 *   \begin{array}{cccc}
 *     y(0), & y(1), & \ldots, & y(T-1).
 *   \end{array}
 * @f]
 *
 * [1] D. Giannoulis, M. Massberg, and J. D. Reiss, &quot;Digital dynamic range
 *     compressor design - A tutorial and analysis,&quot; Journal of the Audio
 *     Engineering Society, vol. 60, no. 6, pp. 399-408, 2012.
 */
class DynamicRangeCompression {
 public:
  /**
   * Detector type.
   */
  enum DetectorType {
    kBranching = 0,
    kDecoupled,
    kBranchingSmooth,
    kDecoupledSmooth,
    kNumDetectorTypes
  };

  /**
   * Buffer for DynamicRangeCompression class.
   */
  class Buffer {
   public:
    Buffer() : prev_y1(0.0), prev_yl(0.0) {
    }

    virtual ~Buffer() {
    }

   private:
    double prev_y1;
    double prev_yl;

    friend class DynamicRangeCompression;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] abs_max_value Absolute maximum value.
   * @param[in] sampling_rate Sampling rate in Hz.
   * @param[in] threshold Threshold in dB.
   * @param[in] ratio Input/output ratio.
   * @param[in] knee_width Knee width in dB.
   * @param[in] attack_time Attack time in msec.
   * @param[in] release_time Release time in msec.
   * @param[in] makeup_gain Make-up gain to compensate output loudness.
   * @param[in] detector_type Detector type (optional).
   */
  DynamicRangeCompression(
      double abs_max_value, double sampling_rate, double threshold,
      double ratio, double knee_width, double attack_time, double release_time,
      double makeup_gain,
      DetectorType detector_type = DetectorType::kDecoupled);

  virtual ~DynamicRangeCompression() {
  }

  /**
   * @return Absolute maximum value.
   */
  double GetAbsMaxValue() const {
    return abs_max_value_;
  }

  /**
   * @return Sampling rate.
   */
  double GetSamplingRate() const {
    return sampling_rate_;
  }

  /**
   * @return Threshold.
   */
  double GetThreshold() const {
    return threshold_;
  }

  /**
   * @return Ratio.
   */
  double GetRatio() const {
    return ratio_;
  }

  /**
   * @return Knee width.
   */
  double GetKneeWidth() const {
    return knee_width_;
  }

  /**
   * @return Attack time.
   */
  double GetAttackTime() const {
    return attack_time_;
  }

  /**
   * @return Release time.
   */
  double GetReleaseTime() const {
    return release_time_;
  }

  /**
   * @return Makeup gain.
   */
  double GetMakeupGain() const {
    return makeup_gain_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] input Input signal.
   * @param[out] output Output signal.
   * @param[in,out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const double input, double* output,
           DynamicRangeCompression::Buffer* buffer) const;

  /**
   * @param[in,out] input_and_output Input/output signal.
   * @param[in,out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(double* input_and_output,
           DynamicRangeCompression::Buffer* buffer) const;

 private:
  const double abs_max_value_;
  const double sampling_rate_;
  const double threshold_;
  const double ratio_;
  const double knee_width_;
  const double attack_time_;
  const double release_time_;
  const double makeup_gain_;
  const DetectorType detector_type_;

  const double alpha_a_;
  const double alpha_r_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(DynamicRangeCompression);
};

}  // namespace sptk

#endif  // SPTK_COMPRESSION_DYNAMIC_RANGE_COMPRESSION_H_
