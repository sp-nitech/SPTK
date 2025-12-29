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

#include "SPTK/resampler/r8brain_resampler.h"

#include <algorithm>  // std::copy
#include <cstddef>    // std::size_t
#include <vector>     // std::vector

namespace sptk {

R8brainResampler::R8brainResampler(double input_sampling_rate,
                                   double output_sampling_rate,
                                   int vector_length, int buffer_length,
                                   int quality)
    : vector_length_(vector_length),
      buffer_length_(buffer_length),
      is_valid_(true) {
  if (input_sampling_rate <= 0.0 || output_sampling_rate <= 0.0 ||
      vector_length_ <= 0 || buffer_length_ <= 0 ||
      quality < GetMinimumQuality() || GetMaximumQuality() < quality) {
    is_valid_ = false;
    return;
  }

  for (int i(0); i < vector_length_; ++i) {
    resamplers_.emplace_back(new r8b::CDSPResampler24(
        input_sampling_rate, output_sampling_rate, buffer_length_));
  }
}

bool R8brainResampler::Get(const std::vector<double>& inputs,
                           std::vector<double>* outputs) {
  if (!is_valid_ || inputs.empty() || NULL == outputs) {
    return false;
  }

  const int input_size(static_cast<int>(inputs.size()));
  const int num_input_frames(input_size / vector_length_);
  if (0 != input_size % vector_length_ || buffer_length_ < num_input_frames) {
    return false;
  }

  if (1 == vector_length_) {
    double* tmp_outputs;
    const int num_output_frames(resamplers_[0]->process(
        const_cast<double*>(inputs.data()), num_input_frames, tmp_outputs));
    if (num_output_frames < 0) {
      return false;
    }
    if (outputs->size() != static_cast<std::size_t>(num_output_frames)) {
      outputs->resize(num_output_frames);
    }
    std::copy(tmp_outputs, tmp_outputs + num_output_frames, outputs->begin());
  } else {
    std::vector<double> tmp_inputs(num_input_frames);
    for (int i(0); i < vector_length_; ++i) {
      for (int j(0); j < num_input_frames; ++j) {
        tmp_inputs[j] = inputs[j * vector_length_ + i];
      }

      double* tmp_outputs;
      const int num_output_frames(resamplers_[i]->process(
          tmp_inputs.data(), num_input_frames, tmp_outputs));
      if (num_output_frames < 0) {
        return false;
      }

      if (0 == i) {
        const std::size_t required_size(
            static_cast<std::size_t>(num_output_frames * vector_length_));
        if (outputs->size() != required_size) {
          outputs->resize(required_size);
        }
      }

      for (int j(0); j < num_output_frames; ++j) {
        (*outputs)[j * vector_length_ + i] = tmp_outputs[j];
      }
    }
  }

  return true;
}

}  // namespace sptk
