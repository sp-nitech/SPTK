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

#include "SPTK/generation/delta_calculation.h"

#include <algorithm>  // std::copy, std::fill
#include <cstddef>    // std::size_t

namespace sptk {

DeltaCalculation::DeltaCalculation(
    int num_order, const std::vector<std::vector<double> >& window_coefficients,
    InputSourceInterface* input_source, bool use_magic_number,
    double magic_number)
    : num_order_(num_order),
      num_delta_(static_cast<int>(window_coefficients.size())),
      window_coefficients_(window_coefficients),
      input_source_(input_source),
      use_magic_number_(use_magic_number),
      magic_number_(magic_number),
      is_valid_(true) {
  if (num_order_ < 0 || num_delta_ <= 0 || NULL == input_source_ ||
      !input_source->IsValid() || input_source->GetSize() != num_order_ + 1) {
    is_valid_ = false;
    return;
  }

  max_window_width_ = 0;
  for (int d(0); d < num_delta_; ++d) {
    if (window_coefficients[d].empty()) {
      is_valid_ = false;
      return;
    }
    if (max_window_width_ < static_cast<int>(window_coefficients[d].size())) {
      max_window_width_ = static_cast<int>(window_coefficients[d].size());
    }
  }

  lefts_.resize(num_delta_);
  rights_.resize(num_delta_);
  for (int d(0); d < num_delta_; ++d) {
    lefts_[d] = -static_cast<int>(window_coefficients[d].size() / 2);
    rights_[d] = static_cast<int>(window_coefficients[d].size() / 2);
    if (0 == window_coefficients[d].size() % 2) {
      --rights_[d];
    }
  }

  buffer_.statics.resize(max_window_width_);
  for (int j(0); j < max_window_width_; ++j) {
    buffer_.statics[j].resize(num_order_ + 1);
  }
  buffer_.pointer = 0;
  buffer_.first = true;

  const int right_window_width((max_window_width_ - 1) / 2);
  buffer_.count_down = right_window_width;
  for (int j(0); j < right_window_width; ++j) {
    if (!Forward()) {
      is_valid_ = false;
      return;
    }
  }
}

bool DeltaCalculation::Get(std::vector<double>* dynamics) {
  if (!is_valid_ || NULL == dynamics) {
    return false;
  }

  if (!Forward()) {
    return false;
  }

  // Prepare memories.
  const int input_length(num_order_ + 1);
  const int output_length(GetSize());
  if (dynamics->size() != static_cast<std::size_t>(output_length)) {
    dynamics->resize(output_length);
  }

  std::fill(dynamics->begin(), dynamics->end(), 0.0);

  // Calculate delta components.
  double* output(&((*dynamics)[0]));
  for (int d(0); d < num_delta_; ++d) {
    for (int j(lefts_[d]), i(0); j <= rights_[d]; ++j, ++i) {
      // The below line means GetPointerIndex(j - delay - 1).
      const int k(GetPointerIndex(j - (max_window_width_ + 1) / 2));
      for (int m(0); m <= num_order_; ++m) {
        const int l(m + input_length * d);
        if (use_magic_number_) {
          if (magic_number_ == buffer_.statics[k][m]) {
            output[l] = magic_number_;
          } else if (magic_number_ != output[l]) {
            output[l] += window_coefficients_[d][i] * buffer_.statics[k][m];
          }
        } else {
          output[l] += window_coefficients_[d][i] * buffer_.statics[k][m];
        }
      }
    }
  }

  return true;
}

bool DeltaCalculation::Forward() {
  // Get and store static components.
  if (!input_source_->Get(&buffer_.statics[buffer_.pointer])) {
    if (buffer_.count_down <= 0) {
      return false;
    }
    --buffer_.count_down;
    // Assume that unobserved future data is same as the last data.
    const int prev(GetPointerIndex(-1));
    std::copy(buffer_.statics[prev].begin(), buffer_.statics[prev].end(),
              buffer_.statics[buffer_.pointer].begin());
  }

  if (buffer_.first) {
    // Assume that unobserved past data is same as the beggining data.
    const int left_window_width(max_window_width_ / 2);
    for (int j(1); j <= left_window_width; ++j) {
      const int prev(GetPointerIndex(-j));
      std::copy(buffer_.statics[0].begin(), buffer_.statics[0].end(),
                buffer_.statics[prev].begin());
    }
    buffer_.first = false;
  }

  // Forward pointer of ring buffer.
  buffer_.pointer = GetPointerIndex(1);

  return true;
}

int DeltaCalculation::GetPointerIndex(int move) {
  int index(buffer_.pointer + move);
  if (move < 0) {
    while (index < 0) index += max_window_width_;
  } else if (0 < move) {
    while (max_window_width_ <= index) index -= max_window_width_;
  }
  return index;
}

}  // namespace sptk
