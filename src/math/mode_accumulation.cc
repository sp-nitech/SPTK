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

#include "SPTK/math/mode_accumulation.h"

#include <algorithm>  // std::find_if
#include <cstddef>    // std::size_t
#include <iterator>   // std::advance
#include <map>        // std::multimap
#include <utility>    // std::make_pair, std::pair

namespace sptk {

ModeAccumulation::ModeAccumulation(int num_best)
    : num_best_(num_best), is_valid_(true) {
  if (num_best_ <= 0) {
    is_valid_ = false;
    return;
  }
}

bool ModeAccumulation::GetMode(int rank, const ModeAccumulation::Buffer& buffer,
                               double* value, int* count) const {
  if (rank <= 0 || buffer.maximum_.size() < static_cast<std::size_t>(rank)) {
    return false;
  }

  std::map<int, double>::const_reverse_iterator itr(buffer.maximum_.rbegin());
  std::advance(itr, rank - 1);
  if (NULL != value) {
    *value = itr->second;
  }
  if (NULL != count) {
    *count = itr->first;
  }

  return true;
}

void ModeAccumulation::Clear(ModeAccumulation::Buffer* buffer) const {
  if (NULL != buffer) buffer->Clear();
}

bool ModeAccumulation::Run(double data,
                           ModeAccumulation::Buffer* buffer) const {
  if (!is_valid_ || NULL == buffer) {
    return false;
  }

  // Update count map.
  if (buffer->count_.find(data) == buffer->count_.end()) {
    buffer->count_[data] = 1;
  } else {
    ++(buffer->count_[data]);
  }

  const int count(buffer->count_[data]);
  const std::pair<int, double> new_pair(std::make_pair(count, data));

  std::multimap<int, double>::iterator itr(std::find_if(
      buffer->maximum_.begin(), buffer->maximum_.end(),
      [data](const std::pair<int, double>& x) { return x.second == data; }));

  // Update inverse map.
  if (itr == buffer->maximum_.end()) {
    if (buffer->maximum_.size() < static_cast<std::size_t>(num_best_)) {
      buffer->maximum_.insert(new_pair);
    } else if (buffer->maximum_.begin()->first < count) {
      buffer->maximum_.erase(buffer->maximum_.begin());
      buffer->maximum_.insert(new_pair);
    }
  } else {
    buffer->maximum_.erase(itr);
    buffer->maximum_.insert(new_pair);
  }

  return true;
}

}  // namespace sptk
