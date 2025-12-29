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

#include "SPTK/resampler/resampler.h"

#include "SPTK/resampler/libsamplerate_resampler.h"
#include "SPTK/resampler/r8brain_resampler.h"
#include "SPTK/resampler/speex_resampler.h"

namespace sptk {

Resampler::Resampler(double input_sampling_rate, double output_sampling_rate,
                     int vector_length, int buffer_length, int quality,
                     Algorithms algorithm) {
  switch (algorithm) {
    case kLibsamplerate: {
      resampler_ =
          new LibsamplerateResampler(input_sampling_rate, output_sampling_rate,
                                     vector_length, buffer_length, quality);
      break;
    }
    case kSpeex: {
      resampler_ = new SpeexResampler(input_sampling_rate, output_sampling_rate,
                                      vector_length, buffer_length, quality);
      break;
    }
    case kR8brain: {
      resampler_ =
          new R8brainResampler(input_sampling_rate, output_sampling_rate,
                               vector_length, buffer_length, quality);
      break;
    }
    default: {
      resampler_ = NULL;
      break;
    }
  }

  // This is because calculation of latency may change internal state so
  // we need to call it only once right after construction.
  latency_ = resampler_ ? resampler_->GetLatency() : 0;
}

int Resampler::GetMinimumQuality(Algorithms algorithm) {
  switch (algorithm) {
    case kLibsamplerate: {
      return LibsamplerateResampler::GetMinimumQuality();
    }
    case kSpeex: {
      return SpeexResampler::GetMinimumQuality();
    }
    case kR8brain: {
      return R8brainResampler::GetMinimumQuality();
    }
    default: {
      return 0;
    }
  }
}

int Resampler::GetMaximumQuality(Algorithms algorithm) {
  switch (algorithm) {
    case kLibsamplerate: {
      return LibsamplerateResampler::GetMaximumQuality();
    }
    case kSpeex: {
      return SpeexResampler::GetMaximumQuality();
    }
    case kR8brain: {
      return R8brainResampler::GetMaximumQuality();
    }
    default: {
      return 0;
    }
  }
}

}  // namespace sptk
