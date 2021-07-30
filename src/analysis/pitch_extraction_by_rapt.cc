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
//                1996-2021  Nagoya Institute of Technology          //
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

#include "SPTK/analysis/pitch_extraction_by_rapt.h"

#include <algorithm>  // std::copy, std::fill
#include <cmath>      // std::ceil

#include "Snack/generic/jkGetF0.h"

namespace sptk {

PitchExtractionByRapt::PitchExtractionByRapt(int frame_shift,
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
      (sampling_rate_ <= 6000.0 || 98000.0 <= sampling_rate_) ||
      (lower_f0_ <= 10.0 || upper_f0_ <= lower_f0_) ||
      (voicing_threshold_ < -0.6 || 0.7 < voicing_threshold_)) {
    is_valid_ = false;
    return;
  }
}

bool PitchExtractionByRapt::Get(
    const std::vector<double>& waveform, std::vector<double>* f0,
    std::vector<double>* epochs,
    PitchExtractionInterface::Polarity* polarity) const {
  // Check inputs.
  if (!is_valid_ || waveform.empty()) {
    return false;
  }

  if (NULL != f0) {
    float* tmp_f0;
    int tmp_length;
    if (0 != snack::cGet_f0(waveform, frame_shift_, sampling_rate_, lower_f0_,
                            upper_f0_, voicing_threshold_, &tmp_f0,
                            &tmp_length)) {
      return false;
    }
    const int target_length(static_cast<int>(
        std::ceil(static_cast<double>(waveform.size()) / frame_shift_)));
    if (target_length < tmp_length) {
      tmp_length = target_length;
    }
    f0->resize(target_length);
    std::copy(tmp_f0, tmp_f0 + tmp_length, f0->begin());
    std::fill(f0->begin() + tmp_length, f0->end(), tmp_f0[tmp_length - 1]);
    snack::ckfree(tmp_f0);
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
