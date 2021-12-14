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

#include "SPTK/analysis/pitch_extraction_by_world.h"

#include <algorithm>  // std::copy, std::fill
#include <cmath>      // std::ceil

#include "WORLD/world/dio.h"

namespace sptk {

PitchExtractionByWorld::PitchExtractionByWorld(int frame_shift,
                                               double sampling_rate,
                                               double lower_f0, double upper_f0,
                                               double voicing_threshold)
    : frame_shift_(frame_shift),
      sampling_rate_(sampling_rate),
      lower_f0_(lower_f0),
      upper_f0_(upper_f0),
      voicing_threshold_(voicing_threshold),
      is_valid_(true) {
  if (frame_shift_ <= 0 || sampling_rate_ / 2 <= upper_f0_ ||
      (sampling_rate_ <= 6.0 || 98000.0 <= sampling_rate_) ||
      (lower_f0_ < 10.0 || upper_f0_ <= lower_f0_) ||
      (voicing_threshold_ < 0.02 || 0.2 < voicing_threshold_)) {
    is_valid_ = false;
    return;
  }
}

bool PitchExtractionByWorld::Get(
    const std::vector<double>& waveform, std::vector<double>* f0,
    std::vector<double>* epochs,
    PitchExtractionInterface::Polarity* polarity) const {
  // Check inputs.
  if (!is_valid_ || waveform.empty()) {
    return false;
  }

  if (NULL != f0) {
    world::DioOption option;
    world::InitializeDioOption(&option);

    const double frame_period((1000.0 * frame_shift_) / sampling_rate_);
    option.frame_period = frame_period;
    option.f0_floor = lower_f0_;
    option.f0_ceil = upper_f0_;
    option.allowed_range = voicing_threshold_;

    const int tmp_length(world::GetSamplesForDIO(
        static_cast<int>(sampling_rate_), static_cast<int>(waveform.size()),
        frame_period));
    std::vector<double> time_axis(tmp_length);
    std::vector<double> tmp_f0(tmp_length);
    world::Dio(&(waveform[0]), static_cast<int>(waveform.size()),
               static_cast<int>(sampling_rate_), &option, &(time_axis[0]),
               &(tmp_f0[0]));

    const int target_length(static_cast<int>(
        std::ceil(static_cast<double>(waveform.size()) / frame_shift_)));
    if (target_length < tmp_length) {
      tmp_f0.resize(target_length);
    }
    f0->resize(target_length);
    std::copy(tmp_f0.begin(), tmp_f0.end(), f0->begin());
    std::fill(f0->begin() + tmp_f0.size(), f0->end(), tmp_f0.back());
  }

  if (NULL != epochs) {
    // nothing to do
  }

  if (NULL != polarity) {
    // nothing to do
  }

  return true;
}

}  // namespace sptk
