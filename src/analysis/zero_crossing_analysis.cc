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

#include "SPTK/analysis/zero_crossing_analysis.h"

#include <cstddef>  // std::size_t

namespace {

bool IsCrossed(double left_sample, double right_sample) {
  return ((0.0 <= left_sample && right_sample < 0.0) ||
          (left_sample < 0.0 && 0.0 <= right_sample));
}

}  // namespace

namespace sptk {

ZeroCrossingAnalysis::ZeroCrossingAnalysis(int frame_length)
    : frame_length_(frame_length), is_valid_(true) {
  if (frame_length <= 0) {
    is_valid_ = false;
    return;
  }
}

bool ZeroCrossingAnalysis::Run(const std::vector<double>& signals,
                               int* num_zero_crossing,
                               ZeroCrossingAnalysis::Buffer* buffer) const {
  // Check inputs.
  if (!is_valid_ || signals.size() != static_cast<std::size_t>(frame_length_) ||
      NULL == num_zero_crossing || NULL == buffer) {
    return false;
  }

  const double* x(&(signals[0]));

  if (buffer->is_first_frame_) {
    buffer->latest_signal_ = x[0];
    buffer->is_first_frame_ = false;
  }

  int count(0);
  if (IsCrossed(buffer->latest_signal_, x[0])) {
    ++count;
  }
  for (int i(1); i < frame_length_; ++i) {
    if (IsCrossed(x[i - 1], x[i])) {
      ++count;
    }
  }

  buffer->latest_signal_ = x[frame_length_ - 1];
  *num_zero_crossing = count;

  return true;
}

}  // namespace sptk
