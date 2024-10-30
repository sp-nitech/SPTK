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

#include "SPTK/math/minmax_accumulation.h"

#include <cstddef>   // std::size_t
#include <iterator>  // std::advance
#include <map>       // std::multimap
#include <utility>   // std::make_pair, std::pair

namespace sptk {

MinMaxAccumulation::MinMaxAccumulation(int num_best)
    : num_best_(num_best), is_valid_(true) {
  if (num_best_ <= 0) {
    is_valid_ = false;
    return;
  }
}

bool MinMaxAccumulation::GetMinimum(int rank,
                                    const MinMaxAccumulation::Buffer& buffer,
                                    double* value, int* position) const {
  if (rank <= 0 || buffer.minimum_.size() < static_cast<std::size_t>(rank)) {
    return false;
  }
  std::multimap<double, int>::const_reverse_iterator itr(
      buffer.minimum_.rbegin());
  std::advance(itr, rank - 1);
  if (NULL != value) {
    *value = itr->first;
  }
  if (NULL != position) {
    *position = itr->second;
  }
  return true;
}

bool MinMaxAccumulation::GetMaximum(int rank,
                                    const MinMaxAccumulation::Buffer& buffer,
                                    double* value, int* position) const {
  if (rank <= 0 || buffer.maximum_.size() < static_cast<std::size_t>(rank)) {
    return false;
  }
  std::multimap<double, int>::const_reverse_iterator itr(
      buffer.maximum_.rbegin());
  std::advance(itr, rank - 1);
  if (NULL != value) {
    *value = itr->first;
  }
  if (NULL != position) {
    *position = itr->second;
  }
  return true;
}

void MinMaxAccumulation::Clear(MinMaxAccumulation::Buffer* buffer) const {
  if (NULL != buffer) buffer->Clear();
}

bool MinMaxAccumulation::Run(double data,
                             MinMaxAccumulation::Buffer* buffer) const {
  if (!is_valid_ || NULL == buffer) {
    return false;
  }

  const std::pair<double, int> new_pair(
      std::make_pair(data, buffer->position_));

  if (buffer->minimum_.size() < static_cast<std::size_t>(num_best_)) {
    buffer->minimum_.insert(new_pair);
  } else if (data < buffer->minimum_.begin()->first) {
    buffer->minimum_.erase(buffer->minimum_.begin());
    buffer->minimum_.insert(new_pair);
  }

  if (buffer->maximum_.size() < static_cast<std::size_t>(num_best_)) {
    buffer->maximum_.insert(new_pair);
  } else if (buffer->maximum_.begin()->first < data) {
    buffer->maximum_.erase(buffer->maximum_.begin());
    buffer->maximum_.insert(new_pair);
  }

  ++(buffer->position_);

  return true;
}

}  // namespace sptk
