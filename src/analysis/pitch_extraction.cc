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

#include "SPTK/analysis/pitch_extraction.h"

#include <vector>  // std::vector

#include "SPTK/analysis/pitch_extraction_by_dio.h"
#include "SPTK/analysis/pitch_extraction_by_harvest.h"
#include "SPTK/analysis/pitch_extraction_by_rapt.h"
#include "SPTK/analysis/pitch_extraction_by_reaper.h"
#include "SPTK/analysis/pitch_extraction_by_swipe.h"

namespace sptk {

PitchExtraction::PitchExtraction(int frame_shift, double sampling_rate,
                                 double lower_f0, double upper_f0,
                                 double voicing_threshold,
                                 PitchExtraction::Algorithms algorithm) {
  switch (algorithm) {
    case kRapt: {
      pitch_extraction_ = new PitchExtractionByRapt(
          frame_shift, sampling_rate, lower_f0, upper_f0, voicing_threshold);
      break;
    }
    case kSwipe: {
      pitch_extraction_ = new PitchExtractionBySwipe(
          frame_shift, sampling_rate, lower_f0, upper_f0, voicing_threshold);
      break;
    }
    case kReaper: {
      pitch_extraction_ = new PitchExtractionByReaper(
          frame_shift, sampling_rate, lower_f0, upper_f0, voicing_threshold);
      break;
    }
    case kDio: {
      pitch_extraction_ = new PitchExtractionByDio(
          frame_shift, sampling_rate, lower_f0, upper_f0, voicing_threshold);
      break;
    }
    case kHarvest: {
      pitch_extraction_ = new PitchExtractionByHarvest(
          frame_shift, sampling_rate, lower_f0, upper_f0, voicing_threshold);
      break;
    }
    default: {
      pitch_extraction_ = NULL;
      break;
    }
  }
}

bool PitchExtraction::Run(const std::vector<double>& waveform,
                          std::vector<double>* f0, std::vector<double>* epochs,
                          PitchExtractionInterface::Polarity* polarity) const {
  if (!IsValid()) {
    return false;
  }

  return pitch_extraction_->Get(waveform, f0, epochs, polarity);
}

}  // namespace sptk
