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

#ifndef SPTK_RESAMPLER_VECTOR_RESAMPLER_H_
#define SPTK_RESAMPLER_VECTOR_RESAMPLER_H_

#include <vector>  // std::vector

#include "SPTK/resampler/scalar_resampler.h"

namespace sptk {

/**
 * Resample vector sequence.
 *
 * The input is the sequence of vectors and the output is the resampled
 * sequence of vectors.
 */
class VectorResampler {
 public:
  /**
   * @param[in] input_sampling_rate Input sampling rate in Hz.
   * @param[in] output_sampling_rate Output sampling rate in Hz.
   * @param[in] algorithm Resampling algorithm.
   */
  VectorResampler(double input_sampling_rate, double output_sampling_rate,
                  ScalarResampler::Algorithms algorithm);

  virtual ~VectorResampler() {
    delete resampler_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return (NULL != resampler_ && resampler_->IsValid());
  }

  /**
   * @param[in] input_vectors Input vectors.
   * @param[out] output_vectors Output vectors.
   * @return True on success, false on failure.
   */
  bool Get(const std::vector<std::vector<double> >& input_vectors,
           std::vector<std::vector<double> >* output_vector) const;
  };

 private:
  ScalarResampler* resampler_;

  DISALLOW_COPY_AND_ASSIGN(VectorResampler);
};

}  // namespace sptk

#endif  // SPTK_RESAMPLER_VECTOR_RESAMPLER_H_
