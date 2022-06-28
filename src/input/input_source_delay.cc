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

#include "SPTK/input/input_source_delay.h"

#include <algorithm>  // std::fill
#include <cstddef>    // std::size_t

namespace sptk {

InputSourceDelay::InputSourceDelay(int delay, bool keep_sequence_length,
                                   InputSourceInterface* source)
    : delay_(delay),
      keep_sequence_length_(keep_sequence_length),
      source_(source),
      is_valid_(true) {
  if (NULL == source_ || !source_->IsValid()) {
    is_valid_ = false;
    return;
  }

  std::vector<double> data;
  if (delay_ <= 0) {
    // Skip data.
    for (num_zeros_ = 0; num_zeros_ < -delay; ++num_zeros_) {
      if (!source_->Get(&data)) {
        break;
      }
    }
  } else {
    // Store data.
    for (num_zeros_ = 0; num_zeros_ < delay_; ++num_zeros_) {
      if (!source_->Get(&data)) {
        break;
      }
      queue_.push(data);
    }
    if (!keep_sequence_length_) {
      num_zeros_ = delay_;
    }
  }
}

bool InputSourceDelay::Get(std::vector<double>* buffer) {
  if (NULL == buffer || !is_valid_) {
    return false;
  }

  if (buffer->size() != static_cast<std::size_t>(GetSize())) {
    buffer->resize(GetSize());
  }

  if (delay_ <= 0) {
    if (source_->Get(buffer)) {
      return true;
    } else if (keep_sequence_length_ && 0 < num_zeros_--) {
      std::fill(buffer->begin(), buffer->end(), 0.0);
      return true;
    }
  } else {
    if (0 < num_zeros_--) {
      std::fill(buffer->begin(), buffer->end(), 0.0);
      return true;
    } else if (source_->Get(buffer)) {
      queue_.push(*buffer);
      *buffer = queue_.front();
      queue_.pop();
      return true;
    } else if (!keep_sequence_length_ && !queue_.empty()) {
      *buffer = queue_.front();
      queue_.pop();
      return true;
    }
  }

  return false;
}

}  // namespace sptk
