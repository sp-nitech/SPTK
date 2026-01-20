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

#ifndef SPTK_RESAMPLER_RESAMPLER_H_
#define SPTK_RESAMPLER_RESAMPLER_H_

#include <vector>  // std::vector

#include "SPTK/resampler/resampler_interface.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Resample signals.
 *
 * The input is the @f$T \times L@f$ samples:
 * @f[
 *   \begin{array}{ccccccc}
 *     x_1(1), & \ldots, & x_1(L), & \ldots, & x_T(1), \ldots, x_T(L),
 *   \end{array}
 * @f]
 * where @f$L@f$ is the number of channels and @f$T@f$ is the number of frames.
 * The output is the @f$T' \times L@f$ samples:
 * @f[
 *   \begin{array}{ccccccc}
 *     y_1(1), & \ldots, & y_1(L), & \ldots, & y_{T'}(1), \ldots, y_{T'}(L),
 *   \end{array}
 * @f]
 * where @f$T'@f$ is the number of output frames determined by the input/output
 * sampling rates.
 */
class Resampler {
 public:
  /**
   * Resampling algorithms.
   */
  enum Algorithms { kLibsamplerate = 0, kSpeex, kR8brain, kNumAlgorithms };

  /**
   * @param[in] algorithm Resampling algorithm.
   * @return Minimum quality of resampling.
   */
  static int GetMinimumQuality(Algorithms algorithm);

  /**
   * @param[in] algorithm Resampling algorithm.
   * @return Maximum quality of resampling.
   */
  static int GetMaximumQuality(Algorithms algorithm);

  /**
   * @return Minimum resampling ratio.
   */
  static double GetMinimumRatio();

  /**
   * @return Maximum resampling ratio.
   */
  static double GetMaximumRatio();

  /**
   * @param[in] input_sampling_rate Input sampling rate in Hz.
   * @param[in] output_sampling_rate Output sampling rate in Hz.
   * @param[in] vector_length Length of vector (number of channels).
   * @param[in] buffer_length Length of buffer in frames.
   * @param[in] quality Quality of resampling (higher is better).
   * @param[in] algorithm Resampling algorithm.
   */
  Resampler(double input_sampling_rate, double output_sampling_rate,
            int vector_length, int buffer_length, int quality,
            Algorithms algorithm);

  virtual ~Resampler() {
    delete resampler_;
  }

  /**
   * Clear internal state.
   */
  void Clear() {
    if (IsValid()) resampler_->Clear();
  }

  /**
   * @return Latency introduced by resampling.
   */
  int GetLatency() {
    return latency_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return (NULL != resampler_ && resampler_->IsValid());
  }

  /**
   * @param[in] inputs Input samples (interleaved if multi-channel).
   * @param[out] outputs Output samples (interleaved if multi-channel).
   * @return True on success, false on failure.
   */
  virtual bool Get(const std::vector<double>& inputs,
                   std::vector<double>* outputs) {
    return IsValid() ? resampler_->Get(inputs, outputs) : false;
  }

 private:
  ResamplerInterface* resampler_;
  int latency_;

  DISALLOW_COPY_AND_ASSIGN(Resampler);
};

}  // namespace sptk

#endif  // SPTK_RESAMPLER_RESAMPLER_H_
