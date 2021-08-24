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

#include "SPTK/input/input_source_from_array.h"

#include <cstddef>  // std::size_t

namespace sptk {

InputSourceFromArray::InputSourceFromArray(bool zero_padding, int read_size,
                                           int array_size, double* input_array)
    : zero_padding_(zero_padding),
      read_size_(read_size),
      array_size_(array_size),
      input_array_(input_array),
      position_(0),
      is_valid_(true) {
  if (read_size_ <= 0 || array_size_ <= 0 || NULL == input_array_) {
    is_valid_ = false;
    return;
  }
}

bool InputSourceFromArray::Get(std::vector<double>* buffer) {
  if (NULL == buffer || !is_valid_ || array_size_ <= position_) {
    return false;
  }

  const int next_position(position_ + read_size_);
  if (!zero_padding_ && array_size_ < next_position) {
    return false;
  }

  if (buffer->size() != static_cast<std::size_t>(read_size_)) {
    buffer->resize(read_size_);
  }

  if (next_position <= array_size_) {
    for (int i(0); i < read_size_; ++i) {
      (*buffer)[i] = input_array_[position_ + i];
    }
  } else {
    const int num_nonzeros(array_size_ - position_);
    for (int i(0); i < num_nonzeros; ++i) {
      (*buffer)[i] = input_array_[position_ + i];
    }
    for (int i(num_nonzeros); i < read_size_; ++i) {
      (*buffer)[i] = 0.0;
    }
  }

  position_ = next_position;

  return true;
}

}  // namespace sptk
