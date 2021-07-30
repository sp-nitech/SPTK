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

#include "SPTK/conversion/cepstrum_to_negative_derivative_of_phase_spectrum.h"

#include <algorithm>  // std::fill, std::reverse_copy
#include <cstddef>    // std::size_t

namespace sptk {

CepstrumToNegativeDerivativeOfPhaseSpectrum::
    CepstrumToNegativeDerivativeOfPhaseSpectrum(int num_order, int fft_length)
    : num_order_(num_order),
      fast_fourier_transform_(fft_length),
      is_valid_(true) {
  if (num_order_ < 0 || fft_length < 2 * num_order_ ||
      !fast_fourier_transform_.IsValid()) {
    is_valid_ = false;
    return;
  }
}

bool CepstrumToNegativeDerivativeOfPhaseSpectrum::Run(
    const std::vector<double>& cepstrum,
    std::vector<double>* negative_derivative_of_phase_spectrum,
    CepstrumToNegativeDerivativeOfPhaseSpectrum::Buffer* buffer) const {
  // Check inputs.
  if (!is_valid_ ||
      cepstrum.size() != static_cast<std::size_t>(num_order_ + 1) ||
      NULL == negative_derivative_of_phase_spectrum || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  const int fft_length(fast_fourier_transform_.GetFftLength());
  if (buffer->fast_fourier_transform_real_part_.size() !=
      static_cast<std::size_t>(fft_length)) {
    buffer->fast_fourier_transform_real_part_.resize(fft_length);
  }

  const double* c(&(cepstrum[0]));
  double* v(&buffer->fast_fourier_transform_real_part_[0]);

  // Fill the left side of the input with cepstrum.
  v[0] = 0.0;
  for (int m(1); m <= num_order_; ++m) {
    v[m] = 0.5 * m * c[m];
  }
  const int half_fft_length(fft_length / 2);
  std::fill(
      buffer->fast_fourier_transform_real_part_.begin() + num_order_ + 1,
      buffer->fast_fourier_transform_real_part_.begin() + half_fft_length + 1,
      0.0);

  // Fill the right side of the input by mirroring the left side.
  std::reverse_copy(
      buffer->fast_fourier_transform_real_part_.begin() + 1,
      buffer->fast_fourier_transform_real_part_.begin() + half_fft_length + 1,
      buffer->fast_fourier_transform_real_part_.begin() + half_fft_length);

  if (half_fft_length == num_order_) {
    // Double the value at the center because the cepstrum values on both
    // sides are overlapped at the center.
    v[num_order_] *= 2.0;
  }

  if (!fast_fourier_transform_.Run(buffer->fast_fourier_transform_real_part_,
                                   negative_derivative_of_phase_spectrum,
                                   &buffer->fast_fourier_transform_imag_part_,
                                   &buffer->fast_fourier_transform_buffer_)) {
    return false;
  }

  return true;
}

}  // namespace sptk
