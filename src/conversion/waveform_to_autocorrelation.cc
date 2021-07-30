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

#include "SPTK/conversion/waveform_to_autocorrelation.h"

#include <cstddef>  // std::size_t

namespace sptk {

WaveformToAutocorrelation::WaveformToAutocorrelation(int frame_length,
                                                     int num_order)
    : frame_length_(frame_length), num_order_(num_order), is_valid_(true) {
  if (frame_length <= 0 || num_order_ < 0) {
    is_valid_ = false;
    return;
  }
}

bool WaveformToAutocorrelation::Run(
    const std::vector<double>& waveform,
    std::vector<double>* autocorrelation) const {
  // Check inputs.
  if (!is_valid_ ||
      waveform.size() != static_cast<std::size_t>(frame_length_) ||
      NULL == autocorrelation) {
    return false;
  }

  // Prepare memories.
  if (autocorrelation->size() != static_cast<std::size_t>(num_order_ + 1)) {
    autocorrelation->resize(num_order_ + 1);
  }

  const double* x(&(waveform[0]));
  double* r(&((*autocorrelation)[0]));

  // Calculate autocorrelation.
  for (int m(0); m <= num_order_; ++m) {
    double sum(0.0);
    for (int l(0); l < frame_length_ - m; ++l) {
      sum += x[l] * x[l + m];
    }
    r[m] = sum;
  }

  return true;
}

}  // namespace sptk
