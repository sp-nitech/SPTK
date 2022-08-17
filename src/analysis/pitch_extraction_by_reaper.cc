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

#include "SPTK/analysis/pitch_extraction_by_reaper.h"

#include <algorithm>  // std::copy, std::fill
#include <cmath>      // std::ceil
#include <cstdint>    // int16_t

#include "REAPER/epoch_tracker/epoch_tracker.h"

namespace sptk {

PitchExtractionByReaper::PitchExtractionByReaper(int frame_shift,
                                                 double sampling_rate,
                                                 double lower_f0,
                                                 double upper_f0,
                                                 double voicing_threshold)
    : frame_shift_(frame_shift),
      sampling_rate_(sampling_rate),
      lower_f0_(lower_f0),
      upper_f0_(upper_f0),
      voicing_threshold_(voicing_threshold),
      is_valid_(true) {
  if (frame_shift_ <= 0 || sampling_rate_ / 2 <= upper_f0_ ||
      (sampling_rate_ <= reaper::kMinSampleRate || 98000.0 <= sampling_rate_) ||
      (lower_f0_ <= 10.0 || upper_f0_ <= lower_f0_) ||
      (voicing_threshold_ < -0.5 || 1.6 < voicing_threshold_)) {
    is_valid_ = false;
    return;
  }
}

bool PitchExtractionByReaper::Get(
    const std::vector<double>& waveform, std::vector<double>* f0,
    std::vector<double>* epochs,
    PitchExtractionInterface::Polarity* polarity) const {
  // Check inputs.
  if (!is_valid_ || waveform.empty()) {
    return false;
  }

  reaper::EpochTracker epoch_tracker;
  epoch_tracker.set_unvoiced_cost(static_cast<float>(voicing_threshold_));
  std::vector<int16_t> integer_waveform(waveform.begin(), waveform.end());
  if (!epoch_tracker.Init(
          &(integer_waveform[0]), static_cast<int32_t>(waveform.size()),
          static_cast<float>(sampling_rate_), static_cast<float>(lower_f0_),
          static_cast<float>(upper_f0_), true, false)) {
    return false;
  }

  if (NULL != polarity) {
    // ComputePolarity should be run before calling ComputeFeatures.
    int tmp;
    if (!epoch_tracker.ComputePolarity(&tmp)) {
      return false;
    }
    if (1 == tmp) {
      *polarity = PitchExtractionInterface::Polarity::kPositive;
    } else if (-1 == tmp) {
      *polarity = PitchExtractionInterface::Polarity::kNegative;
    } else {
      *polarity = PitchExtractionInterface::Polarity::kUnknown;
    }
  }

  if (NULL != f0 || NULL != epochs) {
    if (!epoch_tracker.ComputeFeatures()) {
      return false;
    }
    const bool track_result(epoch_tracker.TrackEpochs());
    epoch_tracker.WriteDiagnostics("");
    if (!track_result) {
      return false;
    }
  }

  if (NULL != f0) {
    const float external_frame_interval(static_cast<float>(frame_shift_) /
                                        static_cast<float>(sampling_rate_));
    std::vector<float> tmp_f0;
    std::vector<float> correlation;
    if (!epoch_tracker.ResampleAndReturnResults(external_frame_interval,
                                                &tmp_f0, &correlation)) {
      return false;
    }
    const int target_length(static_cast<int>(
        std::ceil(static_cast<float>(waveform.size()) / frame_shift_)));
    if (target_length < static_cast<int>(tmp_f0.size())) {
      tmp_f0.resize(target_length);
    }
    f0->resize(target_length);
    std::copy(tmp_f0.begin(), tmp_f0.end(), f0->begin());
    std::fill(f0->begin() + tmp_f0.size(), f0->end(), tmp_f0.back());
  }

  if (NULL != epochs) {
    std::vector<float> times;
    std::vector<int16_t> voicing;
    epoch_tracker.GetFilledEpochs(reaper::kUnvoicedPulseInterval, &times,
                                  &voicing);

    const int time_length(static_cast<int>(times.size()));
    int num_epochs(0);
    for (int i(0); i < time_length; ++i) {
      if (voicing[i]) ++num_epochs;
    }
    epochs->resize(num_epochs);
    double* output(&((*epochs)[0]));
    for (int i(0), j(0); i < time_length; ++i) {
      if (voicing[i]) output[j++] = times[i];
    }
  }

  return true;
}

}  // namespace sptk
