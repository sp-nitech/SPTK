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

#include "SPTK/analysis/spectrum_extraction.h"

#include <algorithm>  // std::copy, std::fill, std::min
#include <cmath>      // std::ceil
#include <vector>     // std::vector

#include "SPTK/analysis/spectrum_extraction_by_world.h"

namespace sptk {

SpectrumExtraction::SpectrumExtraction(
    int fft_length, int frame_shift, double sampling_rate, bool f0_refinement,
    SpectrumExtraction::Algorithms algorithm) {
  switch (algorithm) {
    case kWorld: {
      spectrum_extraction_ = new SpectrumExtractionByWorld(
          fft_length, frame_shift, sampling_rate, f0_refinement);
      break;
    }
    default: {
      spectrum_extraction_ = NULL;
      break;
    }
  }
}

bool SpectrumExtraction::Run(
    const std::vector<double>& waveform, const std::vector<double>& f0,
    std::vector<std::vector<double> >* spectrum) const {
  if (!IsValid()) {
    return false;
  }

  const int target_f0_length(
      static_cast<int>(std::ceil(static_cast<double>(waveform.size()) /
                                 spectrum_extraction_->GetFrameShift())));
  const int given_f0_length(static_cast<int>(f0.size()));
  if (target_f0_length == given_f0_length) {
    return spectrum_extraction_->Run(waveform, f0, spectrum);
  }

  std::vector<double> length_fixed_f0(target_f0_length);
  std::copy(f0.begin(),
            f0.begin() + std::min(target_f0_length, given_f0_length),
            length_fixed_f0.begin());
  if (1 <= given_f0_length && given_f0_length < target_f0_length) {
    std::fill(length_fixed_f0.begin() + given_f0_length, length_fixed_f0.end(),
              f0.back());
  }
  return spectrum_extraction_->Run(waveform, length_fixed_f0, spectrum);
}

}  // namespace sptk
