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

#include "SPTK/converter/filter_coefficients_to_spectrum.h"

#include <algorithm>   // std::copy, std::fill, std::max, std::max_element, etc.
#include <cfloat>      // DBL_MAX
#include <cmath>       // std::log, std::log10, std::pow, std::sqrt
#include <cstddef>     // std::size_t
#include <functional>  // std::bind, std::divides, std::multiplies, etc.

namespace sptk {

FilterCoefficientsToSpectrum::FilterCoefficientsToSpectrum(
    int num_numerator_order, int num_denominator_order, int fft_length,
    OutputFormats output_format, double epsilon_for_calculating_logarithms,
    double relative_floor_in_decibels)
    : num_numerator_order_(num_numerator_order),
      num_denominator_order_(num_denominator_order),
      fft_length_(fft_length),
      output_format_(output_format),
      epsilon_for_calculating_logarithms_(epsilon_for_calculating_logarithms),
      relative_floor_in_decibels_(relative_floor_in_decibels),
      fast_fourier_transform_(fft_length_),
      is_valid_(true) {
  if (num_numerator_order_ < 0 || num_denominator_order_ < 0 ||
      fft_length_ <= num_numerator_order_ ||
      fft_length_ <= num_denominator_order_ || output_format_ < 0 ||
      kNumOutputFormats <= output_format_ ||
      epsilon_for_calculating_logarithms_ < 0.0 ||
      0.0 <= relative_floor_in_decibels_ ||
      !fast_fourier_transform_.IsValid()) {
    is_valid_ = false;
  }
}

bool FilterCoefficientsToSpectrum::Run(
    const std::vector<double>& numerator_coefficients,
    const std::vector<double>& denominator_coefficients,
    std::vector<double>* spectrum,
    FilterCoefficientsToSpectrum::Buffer* buffer) const {
  // check inputs
  const int numerator_length(num_numerator_order_ + 1);
  const int denominator_length(num_denominator_order_ + 1);
  if (!is_valid_ ||
      numerator_coefficients.size() !=
          static_cast<std::size_t>(numerator_length) ||
      denominator_coefficients.size() !=
          static_cast<std::size_t>(denominator_length) ||
      NULL == spectrum || NULL == buffer) {
    return false;
  }

  // prepare memories
  const int output_length(fft_length_ / 2 + 1);
  if (spectrum->size() != static_cast<std::size_t>(output_length)) {
    spectrum->resize(output_length);
  }
  if (buffer->fast_fourier_transform_input_.size() <
      static_cast<std::size_t>(fft_length_)) {
    buffer->fast_fourier_transform_input_.resize(fft_length_);
  }
  if (buffer->numerator_of_transfer_function_.size() <
      static_cast<std::size_t>(output_length)) {
    buffer->numerator_of_transfer_function_.resize(output_length);
  }
  if (buffer->denominator_of_transfer_function_.size() <
      static_cast<std::size_t>(output_length)) {
    buffer->denominator_of_transfer_function_.resize(output_length);
  }

  // calculate numerators of transfer function
  if (1 != numerator_length) {
    std::copy(numerator_coefficients.begin(), numerator_coefficients.end(),
              buffer->fast_fourier_transform_input_.begin());
    std::fill(buffer->fast_fourier_transform_input_.begin() + numerator_length,
              buffer->fast_fourier_transform_input_.end(), 0.0);
    if (!fast_fourier_transform_.Run(
            buffer->fast_fourier_transform_input_,
            &buffer->fast_fourier_transform_real_output_,
            &buffer->fast_fourier_transform_imaginary_output_,
            &buffer->fast_fourier_transform_buffer_)) {
      return false;
    }

    const double* xr(&buffer->fast_fourier_transform_real_output_[0]);
    const double* xi(&buffer->fast_fourier_transform_imaginary_output_[0]);
    double* x(&buffer->numerator_of_transfer_function_[0]);
    for (int i(0); i < output_length; ++i) {
      x[i] = xr[i] * xr[i] + xi[i] * xi[i];
    }
  }

  // calculate denominators of transfer function
  if (1 != denominator_length) {
    buffer->fast_fourier_transform_input_[0] = 1.0;
    std::copy(denominator_coefficients.begin() + 1,
              denominator_coefficients.end(),
              buffer->fast_fourier_transform_input_.begin() + 1);
    std::fill(
        buffer->fast_fourier_transform_input_.begin() + denominator_length,
        buffer->fast_fourier_transform_input_.end(), 0.0);
    if (!fast_fourier_transform_.Run(
            buffer->fast_fourier_transform_input_,
            &buffer->fast_fourier_transform_real_output_,
            &buffer->fast_fourier_transform_imaginary_output_,
            &buffer->fast_fourier_transform_buffer_)) {
      return false;
    }

    const double* yr(&buffer->fast_fourier_transform_real_output_[0]);
    const double* yi(&buffer->fast_fourier_transform_imaginary_output_[0]);
    double* y(&buffer->denominator_of_transfer_function_[0]);
    for (int i(0); i < output_length; ++i) {
      y[i] = yr[i] * yr[i] + yi[i] * yi[i];
      if (0.0 == y[i]) {
        return false;
      }
    }
  }

  // get gain of transfer function
  const double gain(denominator_coefficients[0] * denominator_coefficients[0]);
  if (0.0 == gain) {
    return false;
  }

  // calculate power spectrum
  if (1 == numerator_length && 1 == denominator_length) {
    const double tmp(gain * numerator_coefficients[0] *
                     numerator_coefficients[0]);
    std::fill(spectrum->begin(), spectrum->end(), tmp);
  } else if (1 == numerator_length && 1 != denominator_length) {
    const double tmp(gain * numerator_coefficients[0] *
                     numerator_coefficients[0]);
    std::transform(
        buffer->denominator_of_transfer_function_.begin(),
        buffer->denominator_of_transfer_function_.begin() + output_length,
        spectrum->begin(),
        std::bind(std::divides<double>(), tmp, std::placeholders::_1));
  } else if (1 != numerator_length && 1 == denominator_length) {
    std::transform(
        buffer->numerator_of_transfer_function_.begin(),
        buffer->numerator_of_transfer_function_.begin() + output_length,
        spectrum->begin(),
        std::bind(std::multiplies<double>(), gain, std::placeholders::_1));
  } else {
    std::transform(
        buffer->numerator_of_transfer_function_.begin(),
        buffer->numerator_of_transfer_function_.begin() + output_length,
        buffer->denominator_of_transfer_function_.begin(), spectrum->begin(),
        [gain](double x, double y) { return gain * x / y; });
  }

  if (kLogAmplitudeSpectrumInDecibels == output_format_ ||
      kLogAmplitudeSpectrum == output_format_) {
    if (0.0 != epsilon_for_calculating_logarithms_) {
      std::transform(
          spectrum->begin(), spectrum->end(), spectrum->begin(),
          std::bind(std::plus<double>(), epsilon_for_calculating_logarithms_,
                    std::placeholders::_1));
    }
    if (-DBL_MAX != relative_floor_in_decibels_) {
      const double max_value_of_power_spectrum(
          *std::max_element(spectrum->begin(), spectrum->end()));
      const double floor_in_power_spectrum(
          max_value_of_power_spectrum *
          std::pow(10.0, 0.1 * relative_floor_in_decibels_));
      std::transform(spectrum->begin(), spectrum->end(), spectrum->begin(),
                     [floor_in_power_spectrum](double x) {
                       return std::max(x, floor_in_power_spectrum);
                     });
    }
  }

  switch (output_format_) {
    case kLogAmplitudeSpectrumInDecibels: {
      std::transform(spectrum->begin(), spectrum->end(), spectrum->begin(),
                     [this](double x) { return 10.0 * std::log10(x); });
      break;
    }
    case kLogAmplitudeSpectrum: {
      std::transform(spectrum->begin(), spectrum->end(), spectrum->begin(),
                     [this](double x) { return 0.5 * std::log(x); });
      break;
    }
    case kAmplitudeSpectrum: {
      std::transform(spectrum->begin(), spectrum->end(), spectrum->begin(),
                     [](double x) { return std::sqrt(x); });
      break;
    }
    case kPowerSpectrum: {
      // nothing to do
      break;
    }
    default: { return false; }
  }

  return true;
}

}  // namespace sptk
