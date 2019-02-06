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
//                1996-2019  Nagoya Institute of Technology          //
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

#include "SPTK/converter/cepstrum_to_negative_derivative_of_phase_spectrum.h"

#include <algorithm>  // std::fill, std::reverse_copy
#include <cstddef>    // std::size_t

namespace sptk {

CepstrumToNegativeDerivativeOfPhaseSpectrum::
    CepstrumToNegativeDerivativeOfPhaseSpectrum(int num_order, int fft_length)
    : num_order_(num_order),
      fast_fourier_transform_(fft_length - 1, fft_length),
      is_valid_(true) {
  if (num_order_ < 0 || fft_length < 2 * num_order_ ||
      !fast_fourier_transform_.IsValid()) {
    is_valid_ = false;
  }
}

bool CepstrumToNegativeDerivativeOfPhaseSpectrum::Run(
    const std::vector<double>& cepstrum,
    std::vector<double>* negative_derivative_of_phase_spectrum,
    CepstrumToNegativeDerivativeOfPhaseSpectrum::Buffer* buffer) const {
  // check inputs
  if (!is_valid_ ||
      cepstrum.size() != static_cast<std::size_t>(num_order_ + 1) ||
      NULL == negative_derivative_of_phase_spectrum || NULL == buffer) {
    return false;
  }

  // prepare memories
  const int fft_length(fast_fourier_transform_.GetFftLength());
  if (buffer->fast_fourier_transform_input_.size() !=
      static_cast<std::size_t>(fft_length)) {
    buffer->fast_fourier_transform_input_.resize(fft_length);
  }

  const double* input(&(cepstrum[0]));
  double* fast_fourier_transform_input(
      &buffer->fast_fourier_transform_input_[0]);

  // set a real part input of the fast Fourier transform
  {
    const int half_fft_length(fft_length / 2);
    // fill the left side of the input with cepstrum
    fast_fourier_transform_input[0] = 0.0;
    for (int i(1); i <= num_order_; ++i) {
      fast_fourier_transform_input[i] = 0.5 * input[i] * i;
    }
    std::fill(
        buffer->fast_fourier_transform_input_.begin() + num_order_ + 1,
        buffer->fast_fourier_transform_input_.begin() + half_fft_length + 1,
        0.0);
    // fill the right side of the input by mirroring the left one
    std::reverse_copy(
        buffer->fast_fourier_transform_input_.begin() + 1,
        buffer->fast_fourier_transform_input_.begin() + half_fft_length + 1,
        buffer->fast_fourier_transform_input_.begin() + half_fft_length);

    if (half_fft_length == num_order_) {
      // double the center value of the input because the cepstrum values on
      // both sides are overlapped at the center
      fast_fourier_transform_input[num_order_] *= 2.0;
    }
  }

  if (!fast_fourier_transform_.Run(
          buffer->fast_fourier_transform_input_,
          negative_derivative_of_phase_spectrum,
          &buffer->fast_fourier_transform_imaginary_part_output_,
          &buffer->fast_fourier_transform_buffer_)) {
    return false;
  }

  return true;
}

}  // namespace sptk
