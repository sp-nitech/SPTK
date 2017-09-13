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

#include "SPTK/converter/negative_derivative_of_phase_spectrum_to_cepstrum.h"

#include <algorithm>  // std::copy, std::reverse_copy
#include <cstddef>    // std::size_t

namespace sptk {

NegativeDerivativeOfPhaseSpectrumToCepstrum::
    NegativeDerivativeOfPhaseSpectrumToCepstrum(int fft_size, int num_order)
    : num_order_(num_order),
      fast_fourier_transform_(fft_size - 1, fft_size),
      is_valid_(true) {
  if (num_order < 0 || fft_size < 2 * num_order ||
      !fast_fourier_transform_.IsValid()) {
    is_valid_ = false;
  }
}

bool NegativeDerivativeOfPhaseSpectrumToCepstrum::Run(
    const std::vector<double>& negative_derivative_of_phase_spectrum,
    std::vector<double>* cepstrum,
    NegativeDerivativeOfPhaseSpectrumToCepstrum::Buffer* buffer) const {
  // check inputs
  const int fft_size(fast_fourier_transform_.GetFftSize());
  const int half_fft_size(fft_size / 2);
  if (!is_valid_ ||
      negative_derivative_of_phase_spectrum.size() !=
          static_cast<std::size_t>(half_fft_size + 1) ||
      NULL == cepstrum || NULL == buffer) {
    return false;
  }

  // prepare memories
  if (cepstrum->size() < static_cast<std::size_t>(num_order_ + 1)) {
    cepstrum->resize(num_order_ + 1);
  }
  if (buffer->fast_fourier_transform_input_.size() <
      static_cast<std::size_t>(fft_size)) {
    buffer->fast_fourier_transform_input_.resize(fft_size);
  }

  // set a real part input of the fast Fourier transform
  std::copy(negative_derivative_of_phase_spectrum.begin(),
            negative_derivative_of_phase_spectrum.end(),
            buffer->fast_fourier_transform_input_.begin());
  std::reverse_copy(
      negative_derivative_of_phase_spectrum.begin() + 1,
      negative_derivative_of_phase_spectrum.end(),
      buffer->fast_fourier_transform_input_.begin() + half_fft_size);

  if (!fast_fourier_transform_.Run(
          buffer->fast_fourier_transform_input_,
          &buffer->fast_fourier_transform_real_part_output_,
          &buffer->fast_fourier_transform_imaginary_part_output_,
          &buffer->fast_fourier_transform_buffer_)) {
    return false;
  }

  double* output(&((*cepstrum)[0]));
  double* fast_fourier_transform_real_part_output(
      &buffer->fast_fourier_transform_real_part_output_[0]);

  output[0] = fast_fourier_transform_real_part_output[0];
  for (int i(1); i <= num_order_; ++i) {
    output[i] =
        fast_fourier_transform_real_part_output[i] / (i * half_fft_size);
  }
  if (half_fft_size == num_order_) {
    output[num_order_] *= 0.5;
  }

  return true;
}

}  // namespace sptk
