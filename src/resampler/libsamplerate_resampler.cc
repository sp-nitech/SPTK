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

#include "SPTK/resampler/libsamplerate_resampler.h"

#include <algorithm>  // std::transform
#include <cmath>      // std::ceil
#include <cstddef>    // std::size_t
#include <vector>     // std::vector

#include "libsamplerate/common.h"
#include "libsamplerate/samplerate.h"

namespace {

const int kMarginForSafety(1);

}  // namespace

namespace sptk {

LibsamplerateResampler::LibsamplerateResampler(double input_sampling_rate,
                                               double output_sampling_rate,
                                               int vector_length,
                                               int buffer_length, int quality)
    : input_sampling_rate_(input_sampling_rate),
      output_sampling_rate_(output_sampling_rate),
      vector_length_(vector_length),
      buffer_length_(buffer_length),
      state_(NULL),
      is_valid_(true) {
  if (input_sampling_rate_ <= 0.0 || output_sampling_rate_ <= 0.0 ||
      SRC_MAX_RATIO < output_sampling_rate_ / input_sampling_rate_ ||
      vector_length_ <= 0 || buffer_length_ <= 0 ||
      quality < GetMinimumQuality() || GetMaximumQuality() < quality) {
    is_valid_ = false;
    return;
  }

  int error;
  state_ =
      libsamplerate::src_new(InvertQuality(quality), vector_length_, &error);

  if (NULL == state_ || libsamplerate::SRC_ERR_NO_ERROR != error) {
    is_valid_ = false;
    return;
  }
}

int LibsamplerateResampler::GetLatency() {
  if (!is_valid_) {
    return 0;
  }
  Clear();
  std::vector<double> dummy_input(vector_length_);
  std::vector<double> dummy_output;
  int i(0);
  while (Get(dummy_input, &dummy_output) && dummy_output.empty()) {
    ++i;
  }
  Clear();
  return i / vector_length_;
}

bool LibsamplerateResampler::Get(const std::vector<double>& inputs,
                                 std::vector<double>* outputs) {
  if (!is_valid_ || inputs.empty() || NULL == outputs) {
    return false;
  }

  const int input_size(static_cast<int>(inputs.size()));
  const int num_input_frames(input_size / vector_length_);
  if (0 != input_size % vector_length_ || buffer_length_ < num_input_frames) {
    return false;
  }

  const double ratio(output_sampling_rate_ / input_sampling_rate_);
  const int expected_num_output_frames(
      static_cast<int>(std::ceil(num_input_frames * ratio) + kMarginForSafety));

  if (input_buffer_.size() != static_cast<std::size_t>(input_size)) {
    input_buffer_.resize(input_size);
  }
  if (output_buffer_.size() !=
      static_cast<std::size_t>(expected_num_output_frames * vector_length_)) {
    output_buffer_.resize(expected_num_output_frames * vector_length_);
  }

  std::transform(inputs.begin(), inputs.end(), input_buffer_.begin(),
                 [](double x) { return static_cast<float>(x); });

  libsamplerate::SRC_DATA src_data;
  src_data.data_in = input_buffer_.data();
  src_data.data_out = output_buffer_.data();
  src_data.input_frames = num_input_frames;
  src_data.output_frames = expected_num_output_frames;
  src_data.end_of_input = 0;
  src_data.src_ratio = ratio;

  if (libsamplerate::SRC_ERR_NO_ERROR !=
      libsamplerate::src_process(state_, &src_data)) {
    return false;
  }

  if (src_data.input_frames_used != num_input_frames) {
    return false;
  }

  const int output_size(src_data.output_frames_gen * vector_length_);
  if (outputs->size() != static_cast<std::size_t>(output_size)) {
    outputs->resize(output_size);
  }
  std::transform(output_buffer_.begin(), output_buffer_.begin() + output_size,
                 outputs->begin(),
                 [](float x) { return static_cast<double>(x); });

  return true;
}

}  // namespace sptk
