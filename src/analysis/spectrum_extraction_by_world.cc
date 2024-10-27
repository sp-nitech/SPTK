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

#include "SPTK/analysis/spectrum_extraction_by_world.h"

#include <cstddef>  // std::size_t
#include <vector>   // std::vector

#include "WORLD/world/cheaptrick.h"
#include "WORLD/world/constantnumbers.h"
#include "WORLD/world/stonemask.h"

namespace sptk {

SpectrumExtractionByWorld::SpectrumExtractionByWorld(int fft_length,
                                                     int frame_shift,
                                                     double sampling_rate)
    : fft_length_(fft_length),
      frame_shift_(frame_shift),
      sampling_rate_(sampling_rate),
      is_valid_(true) {
  if (!sptk::IsPowerOfTwo(fft_length_) || fft_length <= 3 ||
      frame_shift_ <= 0 || !sptk::IsInRange(sampling_rate_, 8000.0, 98000.0)) {
    is_valid_ = false;
    return;
  }
}

bool SpectrumExtractionByWorld::Run(
    const std::vector<double>& waveform, const std::vector<double>& f0,
    std::vector<std::vector<double> >* spectrum) const {
  // Check inputs.
  if (!is_valid_ || waveform.empty() || f0.empty() || NULL == spectrum) {
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

  // Check FFT size.
  double f0_floor(world::kFloorF0);
  for (int i(0); i < f0_length; ++i) {
    if (0.0 < f0[i] && f0[i] < f0_floor) {
      f0_floor = f0[i];
    }
  }
  if (f0_floor < world::GetF0FloorForCheapTrick(
                     static_cast<int>(sampling_rate_), fft_length_)) {
    return false;
  }

  const double frame_shift_in_sec(frame_shift_ / sampling_rate_);
  std::vector<double> time_axis(f0_length);
  for (int i(0); i < f0_length; ++i) {
    time_axis[i] = i * frame_shift_in_sec;
  }

  world::CheapTrickOption option;
  world::InitializeCheapTrickOption(static_cast<int>(sampling_rate_), &option);
  option.fft_size = fft_length_;
  option.f0_floor = f0_floor;

  // Prepare memories.
  if (spectrum->size() != static_cast<std::size_t>(f0_length)) {
    spectrum->resize(f0_length);
  }
  const int length(fft_length_ / 2 + 1);
  for (int i(0); i < f0_length; ++i) {
    if ((*spectrum)[i].size() != static_cast<std::size_t>(length)) {
      (*spectrum)[i].resize(length);
    }
  }

  std::vector<double*> pointers;
  pointers.reserve(f0_length);
  for (std::vector<double>& vector : *spectrum) {
    pointers.push_back(vector.data());
  }

  // Modify F0 for pitch-adaptive spectrum estimation.
  std::vector<double> modified_f0(f0_length);
  world::StoneMask(waveform.data(), static_cast<int>(waveform.size()),
                   static_cast<int>(sampling_rate_), time_axis.data(),
                   f0.data(), f0_length, modified_f0.data());

  // Estimate spectrum.
  world::CheapTrick(waveform.data(), static_cast<int>(waveform.size()),
                    static_cast<int>(sampling_rate_), time_axis.data(),
                    modified_f0.data(), f0_length, &option, pointers.data());

  return true;
}

}  // namespace sptk
