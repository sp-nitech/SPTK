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

#include "SPTK/converter/cepstrum_to_autocorrelation.h"

#include <algorithm>   // std::copy, std::transform
#include <cmath>       // std::exp
#include <cstddef>     // std::size_t
#include <functional>  // std::bind1st, std::multiplies

namespace sptk {

CepstrumToAutocorrelation::CepstrumToAutocorrelation(int num_input_order,
                                                     int num_output_order,
                                                     int fft_length)
    : num_input_order_(num_input_order),
      num_output_order_(num_output_order),
      fast_fourier_transform_(fft_length - 1, fft_length),
      is_valid_(true) {
  if (num_input_order_ < 0 || num_output_order_ < 0 ||
      fft_length <= num_input_order_ || fft_length <= num_output_order_ ||
      !fast_fourier_transform_.IsValid()) {
    is_valid_ = false;
  }
}

bool CepstrumToAutocorrelation::Run(
    const std::vector<double>& cepstrum, std::vector<double>* autocorrelation,
    CepstrumToAutocorrelation::Buffer* buffer) const {
  // check inputs
  const int input_length(num_input_order_ + 1);
  if (!is_valid_ || cepstrum.size() != static_cast<std::size_t>(input_length) ||
      NULL == autocorrelation || NULL == buffer) {
    return false;
  }

  // prepare memories
  const int fft_length(fast_fourier_transform_.GetFftLength());
  if (buffer->fast_fourier_transform_time_domain_.size() !=
      static_cast<std::size_t>(fft_length)) {
    buffer->fast_fourier_transform_time_domain_.resize(fft_length);
  }
  const int output_length(num_output_order_ + 1);
  if (autocorrelation->size() != static_cast<std::size_t>(output_length)) {
    autocorrelation->resize(output_length);
  }

  std::copy(cepstrum.begin(), cepstrum.end(),
            buffer->fast_fourier_transform_time_domain_.begin());
  std::fill(buffer->fast_fourier_transform_time_domain_.begin() + input_length,
            buffer->fast_fourier_transform_time_domain_.end(), 0.0);

  if (!fast_fourier_transform_.Run(
          buffer->fast_fourier_transform_time_domain_,
          &buffer->fast_fourier_transform_frequency_domain_,
          &buffer->fast_fourier_transform_imaginary_output_,
          &buffer->fast_fourier_transform_buffer_)) {
    return false;
  }

  std::transform(buffer->fast_fourier_transform_frequency_domain_.begin(),
                 buffer->fast_fourier_transform_frequency_domain_.end(),
                 buffer->fast_fourier_transform_frequency_domain_.begin(),
                 [](double x) { return std::exp(2.0 * x); });

  if (!fast_fourier_transform_.Run(
          buffer->fast_fourier_transform_frequency_domain_,
          &buffer->fast_fourier_transform_time_domain_,
          &buffer->fast_fourier_transform_imaginary_output_,
          &buffer->fast_fourier_transform_buffer_)) {
    return false;
  }

  std::transform(
      buffer->fast_fourier_transform_time_domain_.begin(),
      buffer->fast_fourier_transform_time_domain_.begin() + output_length,
      autocorrelation->begin(),
      std::bind1st(std::multiplies<double>(), 1.0 / fft_length));

  return true;
}

}  // namespace sptk
