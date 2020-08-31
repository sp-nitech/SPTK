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

#include "SPTK/analyzer/fast_fourier_transform_cepstral_analysis.h"

#include <cstddef>  // std::size_t

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
  if (num_order_ < 0 || fft_length < 2 * num_order_ || num_iteration_ <= 0 ||
      acceleration_factor_ < 0.0 || !fast_fourier_transform_.IsValid() ||
      !inverse_fast_fourier_transform_.IsValid()) {
    is_valid_ = false;
  }
}

bool FastFourierTransformCepstralAnalysis::Run(
    const std::vector<double>& log_power_spectrum,
    std::vector<double>* cepstrum,
    FastFourierTransformCepstralAnalysis::Buffer* buffer) const {
  // check inputs
  const int fft_length(fast_fourier_transform_.GetFftLength());
  if (!is_valid_ ||
      log_power_spectrum.size() != static_cast<std::size_t>(fft_length) ||
      NULL == cepstrum || NULL == buffer) {
    return false;
  }

  if (cepstrum->size() != static_cast<std::size_t>(num_order_ + 1)) {
    cepstrum->resize(num_order_ + 1);
  }

  if (!inverse_fast_fourier_transform_.Run(
          log_power_spectrum, &buffer->fast_fourier_transform_time_domain_,
          &buffer->fast_fourier_transform_imaginary_part_output_,
          &buffer->inverse_fast_fourier_transform_buffer_)) {
    return false;
  }

  double* output(&((*cepstrum)[0]));
  double* time_domain_error(&buffer->fast_fourier_transform_time_domain_[0]);
  for (int m(0); m <= num_order_; ++m) {
    output[m] = time_domain_error[m];
    time_domain_error[m] = 0.0;
  }

  for (int n(0); n < num_iteration_; ++n) {
    for (int m(1); m <= num_order_; ++m) {
      time_domain_error[fft_length - m] = time_domain_error[m];
    }

    if (!fast_fourier_transform_.Run(
            buffer->fast_fourier_transform_time_domain_,
            &buffer->fast_fourier_transform_frequency_domain_,
            &buffer->fast_fourier_transform_imaginary_part_output_,
            &buffer->fast_fourier_transform_buffer_)) {
      return false;
    }

    double* frequency_domain_error(
        &buffer->fast_fourier_transform_frequency_domain_[0]);
    for (int k(0); k < fft_length; ++k) {
      if (frequency_domain_error[k] < 0.0) {
        frequency_domain_error[k] = 0.0;
      }
    }

    if (!inverse_fast_fourier_transform_.Run(
            buffer->fast_fourier_transform_frequency_domain_,
            &buffer->fast_fourier_transform_time_domain_,
            &buffer->fast_fourier_transform_imaginary_part_output_,
            &buffer->inverse_fast_fourier_transform_buffer_)) {
      return false;
    }

    for (int m(0); m <= num_order_; ++m) {
      const double t(time_domain_error[m] * (1.0 + acceleration_factor_));
      output[m] += t;
      time_domain_error[m] -= t;
    }
  }

  output[0] *= 0.5;
  if (fft_length / 2 == num_order_) {
    output[num_order_] *= 0.5;
  }

  return true;
}

}  // namespace sptk
