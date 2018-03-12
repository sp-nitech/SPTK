// ----------------------------------------------------------------- //
//             The Speech Signal Processing Toolkit (SPTK)           //
//             developed by SPTK Working Group                       //
//             http://sp-tk.sourceforge.net/                         //
// ----------------------------------------------------------------- //
//                                                                   //
//  Copyright (c) 1984-2007  Tokyo Institute of Technology           //
//                           Interdisciplinary Graduate School of    //
//                           Science and Engineering                 //
//                                                                   //
//                1996-2018  Nagoya Institute of Technology          //
//                           Department of Computer Science          //
//                                                                   //
// All rights reserved.                                              //
//                                                                   //
// Redistribution and use in source and binary forms, with or        //
// without modification, are permitted provided that the following   //
// conditions are met:                                               //
//                                                                   //
// - Redistributions of source code must retain the above copyright  //
//   notice, this list of conditions and the following disclaimer.   //
// - Redistributions in binary form must reproduce the above         //
//   copyright notice, this list of conditions and the following     //
//   disclaimer in the documentation and/or other materials provided //
//   with the distribution.                                          //
// - Neither the name of the SPTK working group nor the names of its //
//   contributors may be used to endorse or promote products derived //
//   from this software without specific prior written permission.   //
//                                                                   //
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            //
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       //
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          //
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          //
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS //
// BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          //
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   //
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     //
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON //
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   //
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    //
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           //
// POSSIBILITY OF SUCH DAMAGE.                                       //
// ----------------------------------------------------------------- //

#include "SPTK/analyzer/pitch_extraction_by_world.h"

#include <algorithm>  // std::copy, std::fill
#include <cmath>      // std::ceil

#include "WORLD/src/world/dio.h"

namespace sptk {

PitchExtractionByWorld::PitchExtractionByWorld(int frame_shift,
                                               double sampling_rate,
                                               double minimum_f0,
                                               double maximum_f0,
                                               double voicing_threshold)
    : frame_shift_(frame_shift),
      sampling_rate_(sampling_rate),
      minimum_f0_(minimum_f0),
      maximum_f0_(maximum_f0),
      voicing_threshold_(voicing_threshold),
      is_valid_(true) {
  if (frame_shift_ <= 0 || sampling_rate_ / 2 <= maximum_f0_ ||
      (sampling_rate_ <= 6.0 || 98000.0 <= sampling_rate_) ||
      (minimum_f0_ < 10.0 || maximum_f0_ <= minimum_f0_) ||
      (voicing_threshold_ < 0.02 || 0.2 < voicing_threshold_)) {
    is_valid_ = false;
  }
}

bool PitchExtractionByWorld::Get(
    const std::vector<double>& waveform, std::vector<double>* f0,
    std::vector<double>* epochs,
    PitchExtractionInterface::Polarity* polarity) const {
  if (!is_valid_ || waveform.empty() ||
      static_cast<int>(waveform.size()) < frame_shift_) {
    return false;
  }

  if (NULL != f0) {
    world::DioOption option({0});
    world::InitializeDioOption(&option);

    const double frame_period((1000.0 * frame_shift_) / sampling_rate_);
    option.frame_period = frame_period;
    option.f0_floor = minimum_f0_;
    option.f0_ceil = maximum_f0_;
    option.allowed_range = voicing_threshold_;

    const int tmp_length(
        world::GetSamplesForDIO(sampling_rate_, waveform.size(), frame_period));
    std::vector<double> time_axis(tmp_length);
    std::vector<double> tmp_f0(tmp_length);
    world::Dio(&(waveform[0]), waveform.size(), sampling_rate_, &option,
               &(time_axis[0]), &(tmp_f0[0]));

    const int target_length(
        std::ceil(static_cast<double>(waveform.size()) / frame_shift_));
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
