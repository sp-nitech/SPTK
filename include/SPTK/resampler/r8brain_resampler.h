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

#ifndef SPTK_RESAMPLER_R8BRAIN_RESAMPLER_H_
#define SPTK_RESAMPLER_R8BRAIN_RESAMPLER_H_

#include <memory>  // std::unique_ptr
#include <vector>  // std::vector

#include "SPTK/resampler/resampler_interface.h"
#include "SPTK/utils/sptk_utils.h"

#ifndef CPPCHECK
#include "r8brain/CDSPResampler.h"
#endif  // CPPCHECK

namespace sptk {

/**
 * Resample data sequence using r8brain library.
 */
class R8brainResampler : public ResamplerInterface {
 public:
  /**
   * @return Minimum quality of resampling.
   */
  static int GetMinimumQuality() {
    return 0;
  }

  /**
   * @return Maximum quality of resampling.
   */
  static int GetMaximumQuality() {
    return 0;
  }

  /**
   * @param[in] input_sampling_rate Input sampling rate in Hz.
   * @param[in] output_sampling_rate Output sampling rate in Hz.
   * @param[in] vector_length Length of input vector (number of channels).
   * @param[in] buffer_length Length of buffer used in resampling.
   * @param[in] quality Quality of resampling.
   */
  R8brainResampler(double input_sampling_rate, double output_sampling_rate,
                   int vector_length, int buffer_length, int quality);

  ~R8brainResampler() override {
  }

  /**
   * Clear internal state.
   */
  void Clear() override {
    for (const std::unique_ptr<r8b::CDSPResampler24>& resampler : resamplers_) {
      resampler->clear();
    }
  }

  /**
   * @return Latency of this resampler in samples.
   */
  int GetLatency() override {
    return IsValid() ? resamplers_[0]->getInLenBeforeOutPos(0) : 0;
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
  const int vector_length_;
  const int buffer_length_;

  std::vector<std::unique_ptr<r8b::CDSPResampler24> > resamplers_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(R8brainResampler);
};

}  // namespace sptk

#endif  // SPTK_RESAMPLER_R8BRAIN_RESAMPLER_H_
