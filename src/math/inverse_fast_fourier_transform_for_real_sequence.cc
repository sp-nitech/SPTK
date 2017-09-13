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
//                1996-2017  Nagoya Institute of Technology          //
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

#include "SPTK/math/inverse_fast_fourier_transform_for_real_sequence.h"

#include <algorithm>   // std::transform
#include <cstddef>     // std::size_t
#include <functional>  // std::bind1st, std::multiplies

namespace sptk {

InverseFastFourierTransformForRealSequence::
    InverseFastFourierTransformForRealSequence(int num_order, int fft_size)
    : fast_fourier_transform_(num_order, fft_size) {
}

bool InverseFastFourierTransformForRealSequence::Run(
    const std::vector<double>& real_part_input,
    std::vector<double>* real_part_output,
    std::vector<double>* imaginary_part_output,
    InverseFastFourierTransformForRealSequence::Buffer* buffer) const {
  // check inputs
  if (!fast_fourier_transform_.IsValid() ||
      real_part_input.size() !=
          static_cast<std::size_t>(fast_fourier_transform_.GetNumOrder() + 1) ||
      NULL == real_part_output || NULL == imaginary_part_output ||
      NULL == buffer) {
    return false;
  }

  if (fast_fourier_transform_.Run(real_part_input, real_part_output,
                                  imaginary_part_output,
                                  &buffer->fast_fourier_transform_buffer_)) {
    return false;
  }

  const int fft_size(fast_fourier_transform_.GetFftSize());
  const double inverse_fft_size(1.0 / fft_size);
  std::transform(real_part_output->begin(),
                 real_part_output->begin() + fft_size,
                 real_part_output->begin(),
                 std::bind1st(std::multiplies<double>(), inverse_fft_size));
  std::transform(imaginary_part_output->begin(),
                 imaginary_part_output->begin() + fft_size,
                 imaginary_part_output->begin(),
                 std::bind1st(std::multiplies<double>(), inverse_fft_size));

  return true;
}

}  // namespace sptk
