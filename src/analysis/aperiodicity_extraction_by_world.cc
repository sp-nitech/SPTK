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

#include "SPTK/analysis/aperiodicity_extraction_by_world.h"

#include <cstddef>  // std::size_t
#include <vector>   // std::vector

#include "WORLD/world/d4c.h"

namespace sptk {

AperiodicityExtractionByWorld::AperiodicityExtractionByWorld(
    int fft_length, int frame_shift, double sampling_rate)
    : fft_length_(fft_length),
      frame_shift_(frame_shift),
      sampling_rate_(sampling_rate),
      is_valid_(true) {
  if (fft_length <= 0 || frame_shift_ <= 0 ||
      !sptk::IsInRange(sampling_rate_, 8000.0, 98000.0)) {
    is_valid_ = false;
    return;
  }
}

bool AperiodicityExtractionByWorld::Run(
    const std::vector<double>& waveform, const std::vector<double>& f0,
    std::vector<std::vector<double> >* aperiodicity) const {
  // Check inputs.
  if (!is_valid_ || waveform.empty() || f0.empty() || NULL == aperiodicity) {
    return false;
  }

  // Check F0 values to prevent segmentation fault.
  const int f0_length(static_cast<int>(f0.size()));
  const double nyquist_frequency(0.5 * sampling_rate_);
  for (int i(0); i < f0_length; ++i) {
    if (f0[i] < 0.0 || nyquist_frequency < f0[i]) {
      return false;
    }
  }

  const double frame_shift_in_sec(frame_shift_ / sampling_rate_);
  std::vector<double> time_axis(f0_length);
  for (int i(0); i < f0_length; ++i) {
    time_axis[i] = i * frame_shift_in_sec;
  }

  world::D4COption option;
  world::InitializeD4COption(&option);
  option.threshold = 0.0;

  // Prepare memories.
  if (aperiodicity->size() != static_cast<std::size_t>(f0_length)) {
    aperiodicity->resize(f0_length);
  }
  const int length(fft_length_ / 2 + 1);
  for (int i(0); i < f0_length; ++i) {
    if ((*aperiodicity)[i].size() != static_cast<std::size_t>(length)) {
      (*aperiodicity)[i].resize(length);
    }
  }

  std::vector<double*> pointers;
  pointers.reserve(f0_length);
  for (std::vector<double>& vector : *aperiodicity) {
    pointers.push_back(vector.data());
  }

  world::D4C(waveform.data(), static_cast<int>(waveform.size()),
             static_cast<int>(sampling_rate_), time_axis.data(), f0.data(),
             f0_length, fft_length_, &option, pointers.data());

  return true;
}

}  // namespace sptk
