// ----------------------------------------------------------------- //
//             The Speech Signal Processing Toolkit (SPTK)           //
//             developed by SPTK Working Group                       //
//             http://sp-tk.sourceforge.net/                         //
// ----------------------------------------------------------------- //
//                                                                   //
//  Copyright (c) 1984-2007  Tokyo Institute of Technology           //
//                           Interdisciplinary Graduate School of    //
//                           Science and Engineering                 //
//                                                                   //
//                1996-2017  Nagoya Institute of Technology          //
//                           Department of Computer Science          //
//                                                                   //
// All rights reserved.                                              //
//                                                                   //
// Redistribution and use in source and binary forms, with or        //
// without modification, are permitted provided that the following   //
// conditions are met:                                               //
//                                                                   //
// - Redistributions of source code must retain the above copyright  //
//   notice, this list of conditions and the following disclaimer.   //
// - Redistributions in binary form must reproduce the above         //
//   copyright notice, this list of conditions and the following     //
//   disclaimer in the documentation and/or other materials provided //
//   with the distribution.                                          //
// - Neither the name of the SPTK working group nor the names of its //
//   contributors may be used to endorse or promote products derived //
//   from this software without specific prior written permission.   //
//                                                                   //
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            //
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       //
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          //
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          //
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS //
// BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          //
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   //
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     //
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON //
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   //
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    //
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           //
// POSSIBILITY OF SUCH DAMAGE.                                       //
// ----------------------------------------------------------------- //

#include "SPTK/input/input_source_interpolation.h"

#include <algorithm>   // std::copy, std::transform
#include <cstddef>     // std::size_t
#include <functional>  // std::plus

namespace sptk {

InputSourceInterpolation::InputSourceInterpolation(
    int frame_period, int interpolation_period,
    bool use_final_frame_for_exceeded_frame, InputSourceInterface* source)
    : frame_period_(frame_period),
      interpolation_period_(interpolation_period),
      first_interpolation_period_(interpolation_period_ / 2),
      use_final_frame_for_exceeded_frame_(use_final_frame_for_exceeded_frame),
      remained_num_samples_(0),
      data_length_(0),
      point_index_in_frame_(0),
      source_(source),
      is_valid_(true) {
  if (frame_period_ <= 0 || interpolation_period_ < 0 ||
      frame_period_ / 2 < interpolation_period_ || NULL == source_ ||
      !source_->IsValid()) {
    is_valid_ = false;
    return;
  }

  if (!source_->Get(&curr_data_)) {
    remained_num_samples_ = 0;
    return;
  }

  remained_num_samples_ = frame_period_ + 1;
  data_length_ = curr_data_.size();

  if (!source_->Get(&next_data_)) {
    next_data_.resize(data_length_);
    std::copy(curr_data_.begin(), curr_data_.end(), next_data_.begin());
    remained_num_samples_ = 1;
  }

  if (0 < interpolation_period_) {
    increment_.resize(data_length_);
    CalculateIncrement();
  }
}

void InputSourceInterpolation::CalculateIncrement() {
  const double rate(static_cast<double>(interpolation_period_) / frame_period_);
  for (int i(0); i < data_length_; ++i) {
    increment_[i] = rate * (next_data_[i] - curr_data_[i]);
  }
}

bool InputSourceInterpolation::Get(std::vector<double>* buffer) {
  if (NULL == buffer || !is_valid_) {
    return false;
  }

  if (remained_num_samples_ <= 0) {
    return false;
  }

  if (buffer->size() < static_cast<std::size_t>(data_length_)) {
    buffer->resize(data_length_);
  }

  std::copy(curr_data_.begin(), curr_data_.end(), buffer->begin());

  --remained_num_samples_;

  if (remained_num_samples_ <= 0) {
    if (use_final_frame_for_exceeded_frame_) {
      remained_num_samples_ = 1;
    }
    return true;
  }

  // Update internal states for the next call.
  ++point_index_in_frame_;

  if (0 == point_index_in_frame_ % frame_period_) {
    // Update current and next data.
    std::copy(next_data_.begin(), next_data_.end(), curr_data_.begin());
    if (!source_->Get(&next_data_)) {
      // Use the final data until the end of input sequence.
      std::copy(curr_data_.begin(), curr_data_.end(), next_data_.begin());
      remained_num_samples_ = 1;
    } else {
      remained_num_samples_ = frame_period_ + 1;
    }

    if (0 < interpolation_period_) {
      CalculateIncrement();
    }

    // Rewind point index.
    point_index_in_frame_ = 0;
  } else if (0 < interpolation_period_ &&
             0 == ((point_index_in_frame_ + first_interpolation_period_) %
                   interpolation_period_)) {
    // Interpolate adjacent data.
    std::transform(curr_data_.begin(), curr_data_.end(), increment_.begin(),
                   curr_data_.begin(), std::plus<double>());
  } else if (0 == interpolation_period_ &&
             frame_period_ / 2 == point_index_in_frame_) {
    std::copy(next_data_.begin(), next_data_.end(), curr_data_.begin());
  }

  return true;
}

}  // namespace sptk
