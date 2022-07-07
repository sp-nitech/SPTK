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

#include "SPTK/filter/median_filter.h"

#include <algorithm>  // std::sort
#include <cstddef>    // std::size_t

namespace sptk {

MedianFilter::MedianFilter(int num_input_order, int num_filter_order,
                           InputSourceInterface* input_source,
                           bool apply_each_dimension, bool use_magic_number,
                           double magic_number)
    : num_input_order_(num_input_order),
      num_filter_order_(num_filter_order),
      input_source_(input_source),
      apply_each_dimension_(apply_each_dimension),
      use_magic_number_(use_magic_number),
      magic_number_(magic_number),
      is_valid_(true) {
  if (num_input_order_ < 0 || num_filter_order_ < 0 || NULL == input_source_ ||
      !input_source->IsValid() ||
      input_source->GetSize() != num_input_order_ + 1) {
    is_valid_ = false;
    return;
  }

  // Prepare memories.
  buffer_.resize(num_input_order_ + 1);
  flat_.resize(apply_each_dimension_
                   ? (num_filter_order_ + 1)
                   : (num_filter_order_ + 1) * buffer_.size());

  // Look ahead.
  const int future(num_filter_order_ / 2);
  count_down_ = future;
  for (int i(0); i < future; ++i) {
    if (!Forward()) {
      is_valid_ = false;
      return;
    }
  }
}

bool MedianFilter::Get(std::vector<double>* output) {
  if (!is_valid_ || NULL == output) {
    return false;
  }

  if (!Forward()) {
    return false;
  }

  const int output_size(GetSize());
  if (output->size() != static_cast<std::size_t>(output_size)) {
    output->resize(output_size);
  }

  for (int m(0); m < output_size; ++m) {
    int num_magic_numbers(0);
    int num_valid_numbers(0);
    for (const std::vector<double>& input_vector : queue_) {
      for (int n(apply_each_dimension_ ? m : 0);
           n <= (apply_each_dimension_ ? m : num_input_order_); ++n) {
        if (use_magic_number_ && magic_number_ == input_vector[n]) {
          ++num_magic_numbers;
        } else {
          flat_[num_valid_numbers++] = input_vector[n];
        }
      }
    }

    if (num_valid_numbers < num_magic_numbers) {
      (*output)[m] = magic_number_;
      continue;
    }

    std::sort(flat_.begin(), flat_.begin() + num_valid_numbers);
    const int half_size(num_valid_numbers / 2);
    if (IsEven(num_valid_numbers)) {
      (*output)[m] = 0.5 * (flat_[half_size - 1] + flat_[half_size]);
    } else {
      (*output)[m] = flat_[half_size];
    }
  }

  if (count_down_ < num_filter_order_ / 2 ||
      num_filter_order_ + 1 <= static_cast<int>(queue_.size())) {
    queue_.pop_front();
  }

  return true;
}

bool MedianFilter::Forward() {
  if (input_source_->Get(&buffer_)) {
    queue_.push_back(buffer_);
  } else if (1 <= count_down_) {
    --count_down_;
  } else {
    return false;
  }
  return true;
}

}  // namespace sptk
