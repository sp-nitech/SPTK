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
//                1996-2020  Nagoya Institute of Technology          //
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

#include "SPTK/analyzer/fast_fourier_transform_cepstral_analysis.h"

#include <algorithm>  // std::transform
#include <cmath>      // std::log
#include <cstddef>    // std::size_t

namespace sptk {

FastFourierTransformCepstralAnalysis::FastFourierTransformCepstralAnalysis(
    int fft_length, int num_order, int num_iteration,
    double acceleration_factor)
    : num_order_(num_order),
      num_iteration_(num_iteration),
      acceleration_factor_(acceleration_factor),
      fast_fourier_transform_(fft_length),
      inverse_fast_fourier_transform_(fft_length),
      is_valid_(true) {
  if (num_order_ < 0 || fft_length < 2 * num_order_ || num_iteration_ < 0 ||
      acceleration_factor_ < 0.0 || !fast_fourier_transform_.IsValid() ||
      !inverse_fast_fourier_transform_.IsValid()) {
    is_valid_ = false;
    return;
  }
}

bool FastFourierTransformCepstralAnalysis::Run(
    const std::vector<double>& power_spectrum, std::vector<double>* cepstrum,
    FastFourierTransformCepstralAnalysis::Buffer* buffer) const {
  // Check inputs.
  const int fft_length(fast_fourier_transform_.GetFftLength());
  if (!is_valid_ ||
      power_spectrum.size() != static_cast<std::size_t>(fft_length / 2 + 1) ||
      NULL == cepstrum || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  if (cepstrum->size() != static_cast<std::size_t>(num_order_ + 1)) {
    cepstrum->resize(num_order_ + 1);
  }
  if (buffer->real_part_.size() != static_cast<std::size_t>(fft_length)) {
    buffer->real_part_.resize(fft_length);
  }

  // Make full log-power spectrum.
  std::transform(power_spectrum.begin(), power_spectrum.end(),
                 buffer->real_part_.begin(),
                 [](double x) { return std::log(x); });
  std::reverse_copy(buffer->real_part_.begin() + 1,
                    buffer->real_part_.begin() + power_spectrum.size() - 1,
                    buffer->real_part_.begin() + power_spectrum.size());

  // Obtain smoothed cepstrum.
  if (!inverse_fast_fourier_transform_.Run(
          &buffer->real_part_, &buffer->imag_part_,
          &buffer->buffer_for_inverse_fast_fourier_transform_)) {
    return false;
  }

  // Initialize variables.
  double* v(&((*cepstrum)[0]));
  double* e(&buffer->real_part_[0]);
  for (int m(0); m <= num_order_; ++m) {
    v[m] = e[m];
    e[m] = 0.0;
  }

  // Perform the improved cepstral analysis method.
  for (int i(0); i < num_iteration_; ++i) {
    for (int m(1); m <= num_order_; ++m) {
      e[fft_length - m] = e[m];
    }

    // Ek = DFT(em).
    if (!fast_fourier_transform_.Run(
            &buffer->real_part_, &buffer->imag_part_,
            &buffer->buffer_for_fast_fourier_transform_)) {
      return false;
    }

    // Apply function g.
    for (int k(0); k < fft_length; ++k) {
      if (e[k] < 0.0) {
        e[k] = 0.0;
      }
    }

    // em = IDFT(Ek).
    if (!inverse_fast_fourier_transform_.Run(
            &buffer->real_part_, &buffer->imag_part_,
            &buffer->buffer_for_inverse_fast_fourier_transform_)) {
      return false;
    }

    // Update vm and em.
    for (int m(0); m <= num_order_; ++m) {
      const double tm(e[m] * (1.0 + acceleration_factor_));
      v[m] += tm;
      e[m] -= tm;
    }
  }

  v[0] *= 0.5;
  if (fft_length / 2 == num_order_) {
    v[num_order_] *= 0.5;
  }

  return true;
}

}  // namespace sptk
