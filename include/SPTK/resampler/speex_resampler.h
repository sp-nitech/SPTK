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

#ifndef SPTK_RESAMPLER_SPEEX_RESAMPLER_H_
#define SPTK_RESAMPLER_SPEEX_RESAMPLER_H_

#include <vector>  // std::vector

#include "SPTK/resampler/resampler_interface.h"
#include "SPTK/utils/sptk_utils.h"

#ifndef CPPCHECK
#include "SpeexDSP/speex_resampler.h"
#endif  // CPPCHECK

namespace sptk {

/**
 * Resample data sequence using speex library.
 */
class SpeexResampler : public ResamplerInterface {
 public:
  /**
   * @return Minimum quality of resampling.
   */
  static int GetMinimumQuality() {
    return SPEEX_RESAMPLER_QUALITY_MIN;
  }

  /**
   * @return Maximum quality of resampling.
   */
  static int GetMaximumQuality() {
    return SPEEX_RESAMPLER_QUALITY_MAX;
  }

  /**
   * @param[in] input_sampling_rate Input sampling rate in Hz.
   * @param[in] output_sampling_rate Output sampling rate in Hz.
   * @param[in] vector_length Length of vector (number of channels).
   * @param[in] buffer_length Length of buffer used in resampling.
   * @param[in] quality Quality of resampling.
   */
  SpeexResampler(double input_sampling_rate, double output_sampling_rate,
                 int vector_length, int buffer_length, int quality);

  ~SpeexResampler() override {
    if (state_) sptk_speex_resampler_destroy(state_);
  }

  /**
   * Clear internal state.
   */
  void Clear() override {
    if (state_) {
      sptk_speex_resampler_reset_mem(state_);
      sptk_speex_resampler_skip_zeros(state_);
    }
  }

  /**
   * @return Latency of this resampler in samples.
   */
  int GetLatency() override {
    return state_ ? sptk_speex_resampler_get_input_latency(state_) : 0;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const override {
    return is_valid_;
  }

  /**
   * @param[in] inputs Input samples (interleaved if multi-channel).
   * @param[out] outputs Output samples (interleaved if multi-channel).
   * @return True on success, false on failure.
   */
  bool Get(const std::vector<double>& inputs,
           std::vector<double>* outputs) override;

 private:
  const double input_sampling_rate_;
  const double output_sampling_rate_;
  const int vector_length_;
  const int buffer_length_;

  SpeexResamplerState* state_;

  bool is_valid_;

  std::vector<float> input_buffer_;
  std::vector<float> output_buffer_;

  DISALLOW_COPY_AND_ASSIGN(SpeexResampler);
};

}  // namespace sptk

#endif  // SPTK_RESAMPLER_SPEEX_RESAMPLER_H_
