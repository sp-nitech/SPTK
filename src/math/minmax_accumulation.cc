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
//                1996-2020  Nagoya Institute of Technology          //
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
