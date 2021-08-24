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
#include <utility>   // std::make_pair

namespace {

bool CompareToSortInAscendingOrder(const std::pair<int, double>& a,
                                   const std::pair<int, double>& b) {
  return a.second < b.second;
}

bool CompareToSortInDescendingOrder(const std::pair<int, double>& a,
                                    const std::pair<int, double>& b) {
  return b.second < a.second;
}

}  // namespace

namespace sptk {

MinMaxAccumulation::MinMaxAccumulation(int num_best)
    : num_best_(num_best), is_valid_(true) {
  if (num_best_ <= 0) {
    is_valid_ = false;
    return;
  }
}

bool MinMaxAccumulation::GetMinimum(const MinMaxAccumulation::Buffer& buffer,
                                    int rank, int* position,
                                    double* value) const {
  if (rank <= 0 || buffer.minimum_.size() < static_cast<std::size_t>(rank)) {
    return false;
  }
  std::list<std::pair<int, double> >::const_iterator itr(
      buffer.minimum_.begin());
  std::advance(itr, rank - 1);
  if (NULL != position) {
    *position = (*itr).first;
  }
  if (NULL != value) {
    *value = (*itr).second;
  }
  return true;
}

bool MinMaxAccumulation::GetMaximum(const MinMaxAccumulation::Buffer& buffer,
                                    int rank, int* position,
                                    double* value) const {
  if (rank <= 0 || buffer.maximum_.size() < static_cast<std::size_t>(rank)) {
    return false;
  }
  std::list<std::pair<int, double> >::const_iterator itr(
      buffer.maximum_.begin());
  std::advance(itr, rank - 1);
  if (NULL != position) {
    *position = (*itr).first;
  }
  if (NULL != value) {
    *value = (*itr).second;
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

  std::pair<int, double> new_pair(std::make_pair(buffer->position_, data));

  if (buffer->minimum_.size() < static_cast<std::size_t>(num_best_)) {
    buffer->minimum_.push_front(new_pair);
    buffer->minimum_.sort(CompareToSortInAscendingOrder);
  } else if (data <= buffer->minimum_.begin()->second) {
    buffer->minimum_.push_front(new_pair);
    buffer->minimum_.pop_back();
  } else if (data < buffer->minimum_.rbegin()->second) {
    buffer->minimum_.push_front(new_pair);
    buffer->minimum_.sort(CompareToSortInAscendingOrder);
    buffer->minimum_.pop_back();
  }

  if (buffer->maximum_.size() < static_cast<std::size_t>(num_best_)) {
    buffer->maximum_.push_front(new_pair);
    buffer->maximum_.sort(CompareToSortInDescendingOrder);
  } else if (buffer->maximum_.begin()->second <= data) {
    buffer->maximum_.push_front(new_pair);
    buffer->maximum_.pop_back();
  } else if (buffer->maximum_.rbegin()->second < data) {
    buffer->maximum_.push_front(new_pair);
    buffer->maximum_.sort(CompareToSortInDescendingOrder);
    buffer->maximum_.pop_back();
  }

  ++(buffer->position_);

  return true;
}

}  // namespace sptk
