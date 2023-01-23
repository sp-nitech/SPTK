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

#include "SPTK/analysis/spectrum_extraction_by_world.h"

namespace sptk {

SpectrumExtraction::SpectrumExtraction(
    int fft_length, int frame_shift, double sampling_rate,
    SpectrumExtraction::Algorithms algorithm) {
  switch (algorithm) {
    case kWorld: {
      spectrum_extraction_ =
          new SpectrumExtractionByWorld(fft_length, frame_shift, sampling_rate);
      break;
    }
    default: {
      spectrum_extraction_ = NULL;
      break;
    }
  }
}

}  // namespace sptk
