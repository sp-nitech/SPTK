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

#include "SPTK/conversion/waveform_to_autocorrelation.h"

#include <cstddef>  // std::size_t

namespace sptk {

WaveformToAutocorrelation::WaveformToAutocorrelation(int frame_length,
                                                     int num_order)
    : frame_length_(frame_length), num_order_(num_order), is_valid_(true) {
  if (frame_length_ <= 0 || num_order_ < 0 || frame_length_ <= num_order_) {
    is_valid_ = false;
    return;
  }
}

bool WaveformToAutocorrelation::Run(
    const std::vector<double>& waveform,
    std::vector<double>* autocorrelation) const {
  // Check inputs.
  if (!is_valid_ ||
      waveform.size() != static_cast<std::size_t>(frame_length_) ||
      NULL == autocorrelation) {
    return false;
  }

  // Prepare memories.
  if (autocorrelation->size() != static_cast<std::size_t>(num_order_ + 1)) {
    autocorrelation->resize(num_order_ + 1);
  }

  const double* x(&(waveform[0]));
  double* r(&((*autocorrelation)[0]));

  // Calculate autocorrelation.
  for (int m(0); m <= num_order_; ++m) {
    double sum(0.0);
    for (int l(0); l < frame_length_ - m; ++l) {
      sum += x[l] * x[l + m];
    }
    r[m] = sum;
  }

  return true;
}

}  // namespace sptk
