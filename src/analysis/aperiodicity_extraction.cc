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

#include "SPTK/analysis/aperiodicity_extraction.h"

#include <algorithm>  // std::copy, std::fill, std::min
#include <cmath>      // std::ceil
#include <vector>     // std::vector

#include "SPTK/analysis/aperiodicity_extraction_by_tandem.h"
#include "SPTK/analysis/aperiodicity_extraction_by_world.h"

namespace sptk {

AperiodicityExtraction::AperiodicityExtraction(
    int fft_length, int frame_shift, double sampling_rate,
    AperiodicityExtraction::Algorithms algorithm) {
  switch (algorithm) {
    case kTandem: {
      aperiodicity_extraction_ = new AperiodicityExtractionByTandem(
          fft_length, frame_shift, sampling_rate);
      break;
    }
    case kWorld: {
      aperiodicity_extraction_ = new AperiodicityExtractionByWorld(
          fft_length, frame_shift, sampling_rate);
      break;
    }
    default: {
      aperiodicity_extraction_ = NULL;
      break;
    }
  }
}

bool AperiodicityExtraction::Run(
    const std::vector<double>& waveform, const std::vector<double>& f0,
    std::vector<std::vector<double> >* aperiodicity) const {
  if (!IsValid()) {
    return false;
  }

  const int target_f0_length(
      static_cast<int>(std::ceil(static_cast<double>(waveform.size()) /
                                 aperiodicity_extraction_->GetFrameShift())));
  const int given_f0_length(static_cast<int>(f0.size()));
  if (target_f0_length == given_f0_length) {
    return aperiodicity_extraction_->Run(waveform, f0, aperiodicity);
  }

  std::vector<double> length_fixed_f0(target_f0_length);
  std::copy(f0.begin(),
            f0.begin() + std::min(target_f0_length, given_f0_length),
            length_fixed_f0.begin());
  if (1 <= given_f0_length && given_f0_length < target_f0_length) {
    std::fill(length_fixed_f0.begin() + given_f0_length, length_fixed_f0.end(),
              f0.back());
  }
  return aperiodicity_extraction_->Run(waveform, length_fixed_f0, aperiodicity);
}

}  // namespace sptk
