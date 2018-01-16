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

#include "SPTK/math/zero_crossing.h"

#include <cstddef>  // std::size_t

namespace {

bool IsCrossed(double left_sample, double right_sample) {
  return ((0.0 <= left_sample && right_sample < 0.0) ||
          (left_sample < 0.0 && 0.0 <= right_sample));
}

}  // namespace

namespace sptk {

ZeroCrossing::ZeroCrossing(int frame_length)
    : frame_length_(frame_length), is_valid_(true) {
  if (frame_length < 1) {
    is_valid_ = false;
  }
}

bool ZeroCrossing::Run(const std::vector<double>& signals,
                       int* num_zero_crossing,
                       ZeroCrossing::Buffer* buffer) const {
  // check inputs
  if (!is_valid_ || signals.size() != static_cast<std::size_t>(frame_length_) ||
      NULL == num_zero_crossing || NULL == buffer) {
    return false;
  }

  const double* x(&(signals[0]));
  int count(0);

  if (buffer->is_first_frame_) {
    buffer->latest_signal_ = x[0];
    buffer->is_first_frame_ = false;
  }

  if (IsCrossed(buffer->latest_signal_, x[0])) {
    ++count;
  }
  for (int i(1); i < frame_length_; ++i) {
    if (IsCrossed(x[i - 1], x[i])) {
      ++count;
    }
  }

  buffer->latest_signal_ = x[frame_length_ - 1];
  *num_zero_crossing = count;

  return true;
}

}  // namespace sptk
