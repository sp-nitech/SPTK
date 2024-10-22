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

#include "SPTK/filter/world_synthesis.h"

#include <cstddef>  // std::size_t
#include <vector>   // std::vector

#include "WORLD/world/synthesis.h"

namespace sptk {

WorldSynthesis::WorldSynthesis(int fft_length, int frame_shift,
                               double sampling_rate)
    : fft_length_(fft_length),
      frame_shift_(frame_shift),
      sampling_rate_(sampling_rate),
      is_valid_(true) {
  if (!sptk::IsPowerOfTwo(fft_length_) || fft_length_ < 512 ||
      frame_shift_ <= 0 || !sptk::IsInRange(sampling_rate_, 8000.0, 98000.0)) {
    is_valid_ = false;
    return;
  }
}

bool WorldSynthesis::Run(const std::vector<double>& f0,
                         const std::vector<std::vector<double>>& spectrum,
                         const std::vector<std::vector<double>>& aperiodicity,
                         std::vector<double>* waveform) const {
  // Check inputs.
  if (!is_valid_ || NULL == waveform) {
    return false;
  }

  int f0_length(static_cast<int>(f0.size()));
  if (static_cast<int>(spectrum.size()) < f0_length) {
    f0_length = static_cast<int>(spectrum.size());
  }
  if (static_cast<int>(aperiodicity.size()) < f0_length) {
    f0_length = static_cast<int>(aperiodicity.size());
  }
  if (f0_length <= 0) {
    return false;
  }

  // Check F0 values to prevent segmentation fault.
  const double nyquist_frequency(0.5 * sampling_rate_);
  for (int i(0); i < f0_length; ++i) {
    if (f0[i] < 0.0 || nyquist_frequency < f0[i]) {
      return false;
    }
  }

  // Prepare memories.
  const std::size_t spectrum_size(fft_length_ / 2 + 1);
  std::vector<const double*> spectrum_pointers;
  spectrum_pointers.reserve(f0_length);
  for (const std::vector<double>& vector : spectrum) {
    if (vector.size() != spectrum_size) {
      return false;
    }
    spectrum_pointers.push_back(vector.data());
  }
  std::vector<const double*> aperiodicity_pointers;
  aperiodicity_pointers.reserve(f0_length);
  for (const std::vector<double>& vector : aperiodicity) {
    if (vector.size() != spectrum_size) {
      return false;
    }
    aperiodicity_pointers.push_back(vector.data());
  }
  const int waveform_length(f0_length * frame_shift_);
  if (waveform->size() != static_cast<std::size_t>(waveform_length)) {
    waveform->resize(waveform_length);
  }

  // Synthesize waveform.
  const double frame_shift_in_ms(frame_shift_ * 1000.0 / sampling_rate_);
  world::Synthesis(f0.data(), f0_length, spectrum_pointers.data(),
                   aperiodicity_pointers.data(), fft_length_, frame_shift_in_ms,
                   static_cast<int>(sampling_rate_), waveform_length,
                   waveform->data());

  return true;
}

}  // namespace sptk
