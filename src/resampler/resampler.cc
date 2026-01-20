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

namespace {

const double kMaxRatio(256.0);

}  // namespace

namespace sptk {

Resampler::Resampler(double input_sampling_rate, double output_sampling_rate,
                     int vector_length, int buffer_length, int quality,
                     Algorithms algorithm)
    : resampler_(NULL), latency_(0) {
  const double ratio(output_sampling_rate / input_sampling_rate);
  if (ratio < GetMinimumRatio() || GetMaximumRatio() < ratio) {
    return;
  }

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
      return;
    }
  }

  // This is because calculation of latency may change internal state so
  // we need to call it only once right after construction.
  latency_ = resampler_->GetLatency();
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

double Resampler::GetMinimumRatio() {
  return 1.0 / GetMaximumRatio();
}

double Resampler::GetMaximumRatio() {
  return kMaxRatio;
}

}  // namespace sptk
