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

#include "SPTK/input/input_source_interpolation_with_magic_number.h"

#include <algorithm>  // std::copy
#include <cfloat>     // DBL_MAX
#include <cstddef>    // std::size_t

namespace {

static const double kCannotCalculateIncrements(DBL_MAX);

}  // namespace

namespace sptk {

InputSourceInterpolationWithMagicNumber::
    InputSourceInterpolationWithMagicNumber(
        int frame_period, int interpolation_period,
        bool use_final_frame_for_exceeded_frame, double magic_number,
        InputSourceInterface* source)
    : frame_period_(frame_period),
      interpolation_period_(interpolation_period),
      first_interpolation_period_(interpolation_period_ / 2),
      use_final_frame_for_exceeded_frame_(use_final_frame_for_exceeded_frame),
      magic_number_(magic_number),
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
  data_length_ = static_cast<int>(curr_data_.size());

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

void InputSourceInterpolationWithMagicNumber::CalculateIncrement() {
  const double rate(static_cast<double>(interpolation_period_) / frame_period_);
  for (int i(0); i < data_length_; ++i) {
    if (magic_number_ == next_data_[i] || magic_number_ == curr_data_[i]) {
      increment_[i] = kCannotCalculateIncrements;
    } else {
      increment_[i] = rate * (next_data_[i] - curr_data_[i]);
    }
  }
}

bool InputSourceInterpolationWithMagicNumber::Get(std::vector<double>* buffer) {
  if (NULL == buffer || !is_valid_) {
    return false;
  }

  if (remained_num_samples_ <= 0) {
    return false;
  }

  if (buffer->size() != static_cast<std::size_t>(data_length_)) {
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
    // Interpolate adjacent data without magic number.
    for (int i(0); i < data_length_; ++i) {
      if (kCannotCalculateIncrements == increment_[i]) {
        curr_data_[i] = magic_number_;
      } else {
        curr_data_[i] += increment_[i];
      }
    }
  } else if (0 == interpolation_period_ &&
             frame_period_ / 2 == point_index_in_frame_) {
    std::copy(next_data_.begin(), next_data_.end(), curr_data_.begin());
  }

  return true;
}

}  // namespace sptk
