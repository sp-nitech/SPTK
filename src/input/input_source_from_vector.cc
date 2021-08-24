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

#include "SPTK/input/input_source_from_vector.h"

#include <algorithm>  // std::copy, std::fill_n
#include <cstddef>    // std::size_t

namespace sptk {

InputSourceFromVector::InputSourceFromVector(bool zero_padding, int read_size,
                                             std::vector<double>* input_vector)
    : zero_padding_(zero_padding),
      read_size_(read_size),
      vector_size_(
          NULL == input_vector ? 0 : static_cast<int>(input_vector->size())),
      input_vector_(input_vector),
      position_(0),
      is_valid_(true) {
  if (read_size_ <= 0 || NULL == input_vector_) {
    is_valid_ = false;
    return;
  }
}

bool InputSourceFromVector::Get(std::vector<double>* buffer) {
  if (NULL == buffer || !is_valid_ || vector_size_ <= position_) {
    return false;
  }

  const int next_position(position_ + read_size_);
  if (!zero_padding_ && vector_size_ < next_position) {
    return false;
  }

  if (buffer->size() != static_cast<std::size_t>(read_size_)) {
    buffer->resize(read_size_);
  }

  if (next_position <= vector_size_) {
    std::copy(input_vector_->begin() + position_,
              input_vector_->begin() + next_position, buffer->begin());
  } else {
    const int num_nonzeros(vector_size_ - position_);
    const int num_zeros(next_position - vector_size_);
    std::copy(input_vector_->begin() + position_,
              input_vector_->begin() + position_ + num_nonzeros,
              buffer->begin());
    std::fill_n(buffer->begin() + num_nonzeros, num_zeros, 0.0);
  }

  position_ = next_position;

  return true;
}

}  // namespace sptk
