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

#ifndef SPTK_RESAMPLER_SCALAR_RESAMPLER_H_
#define SPTK_RESAMPLER_SCALAR_RESAMPLER_H_

#include "SPTK/resampler/resampler_interface.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Resample scalar sequence.
 *
 * The input is the sequence of scalar values and the output is the resampled
 * sequence of scalar values.
 */
class ScalarResampler {
 public:
  /**
   * Resampling algorithms.
   */
  enum Algorithms {
    kR8brain = 0,
    kNumAlgorithms
  };

  /**
   * @param[in] input_sampling_rate Input sampling rate in Hz.
   * @param[in] output_sampling_rate Output sampling rate in Hz.
   * @param[in] algorithm Resampling algorithm.
   */
  ScalarResampler(double input_sampling_rate, double output_sampling_rate,
                  Algorithms algorithm);

  virtual ~ScalarResampler() {
    delete resampler_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return (NULL != resampler_ && resampler_->IsValid());
  }

  /**
   * @param[in] inputs Input samples.
   * @param[out] outputs Output samples.
   * @return True on success, false on failure.
   */
  virtual bool Run(const std::vector<double>& inputs,
                   std::vector<double>* outputs) const;

 private:
  ResamplerInterface* resampler_;

  DISALLOW_COPY_AND_ASSIGN(ScalarResampler);
};

}  // namespace sptk

#endif  // SPTK_RESAMPLER_SCALAR_RESAMPLER_H_
