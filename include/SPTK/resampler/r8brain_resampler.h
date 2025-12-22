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

#include <vector>  // std::vector

#include "SPTK/resampler/resampler_interface.h"

namespace sptk {

/**
 * Resample data sequence using r8brain library.
 */
class R8brainResampler : public ResamplerInterface {
 public:
  /**
   * @param[in] input_sampling_rate Input sampling rate in Hz.
   * @param[in] output_sampling_rate Output sampling rate in Hz.
   */
  R8brainResampler(double input_sampling_rate,
                   double output_sampling_rate);

  ~R8brainResampler() override {
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const override {
    return is_valid_;
  }

  /**
   * @param[in] inputs Input samples.
   * @param[out] outputs Output samples.
   * @return True on success, false on failure.
   */
  bool Get(const std::vector<double>& inputs,
           std::vector<double>* outputs) const override;

 private:
  r8b::CDSPResampler24 resampler_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(R8brainResampler);
};

}  // namespace sptk

#endif  // SPTK_RESAMPLER_R8BRAIN_RESAMPLER_H_
