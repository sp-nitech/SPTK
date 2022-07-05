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

#include "SPTK/input/input_source_filling_magic_number.h"

#include <algorithm>  // std::find_if
#include <cstddef>    // std::size_t
#include <stdexcept>  // std::runtime_error

namespace {

const char* kErrorMessage(
    "InputSourceFillingMagicNumber: Cannot find non-magic number");

enum {
  kExit = 0,
  kFail,
  kGood,
  kWait,
};

}  // namespace

namespace sptk {

InputSourceFillingMagicNumber::InputSourceFillingMagicNumber(
    double magic_number, InputSourceInterface* source)
    : magic_number_(magic_number),
      source_(source),
      is_valid_(true),
      final_output_(GetSize(), magic_number_),
      magic_number_region_(GetSize(), 0) {
  if (NULL == source_ || !source_->IsValid()) {
    is_valid_ = false;
    return;
  }
}

bool InputSourceFillingMagicNumber::Get(std::vector<double>* buffer) {
  if (NULL == buffer || !is_valid_) {
    return false;
  }

  {
    const std::size_t data_size(static_cast<std::size_t>(GetSize()));
    if (buffer->size() != static_cast<std::size_t>(data_size)) {
      buffer->resize(data_size);
    }
  }

  bool halt(false);
  while (!halt) {
    const int result(Proceed());
    switch (result) {
      case kExit: {
        return false;
      }
      case kFail: {
        throw std::runtime_error(kErrorMessage);
      }
      case kGood: {
        halt = true;
        break;
      }
      case kWait: {
        // nothing to do
        break;
      }
    }
  }

  *buffer = queue_.front();
  final_output_ = queue_.front();
  queue_.pop_front();

  return true;
}

int InputSourceFillingMagicNumber::Proceed() {
  // If magic number is not in the front of queue, you can output the data.
  if (!queue_.empty()) {
    if (std::find_if(queue_.front().begin(), queue_.front().end(),
                     [this](double x) { return magic_number_ == x; }) ==
        queue_.front().end()) {
      return kGood;
    }
  }

  const int data_size(GetSize());

  // Load new data.
  std::vector<double> curr_data;
  if (!source_->Get(&curr_data)) {
    if (queue_.empty()) {
      return kExit;
    }
    for (int i(0); i < data_size; ++i) {
      if (magic_number_ == queue_.front()[i]) {
        if (magic_number_ == final_output_[i]) {
          // This means all the data is magic number.
          return kFail;
        }
        // Replace magic number with final output value.
        for (std::deque<std::vector<double> >::iterator itr(queue_.begin());
             itr != queue_.end(); ++itr) {
          (*itr)[i] = final_output_[i];
        }
      }
    }
    return kGood;
  }

  for (int i(0); i < data_size; ++i) {
    const double right(curr_data[i]);

    if (magic_number_ == right) {
      ++magic_number_region_[i];
    } else if (magic_number_region_[i]) {
      // Interpolate magic numbers.
      double left;
      if (magic_number_ == final_output_[i]) {
        left = right;
      } else if (static_cast<int>(queue_.size()) == magic_number_region_[i]) {
        left = final_output_[i];
      } else {
        left = (*(queue_.rbegin() + magic_number_region_[i]))[i];
      }
      const double slope((left - right) / (magic_number_region_[i] + 1));
      double value(right);
      for (int j(0); j < magic_number_region_[i]; ++j) {
        value += slope;
        (*(queue_.rbegin() + j))[i] = value;
      }
      magic_number_region_[i] = 0;
    }
  }
  queue_.push_back(curr_data);

  return kWait;
}

}  // namespace sptk
