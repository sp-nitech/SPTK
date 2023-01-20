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

}  // namespace sptk
