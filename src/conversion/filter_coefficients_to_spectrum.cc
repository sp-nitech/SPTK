// ------------------------------------------------------------------------ //
// Copyright 2021 SPTK Working Group                                        //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ------------------------------------------------------------------------ //

#include "SPTK/conversion/filter_coefficients_to_spectrum.h"

#include <algorithm>  // std::copy, std::fill, std::transform
#include <cfloat>     // DBL_MAX
#include <cstddef>    // std::size_t

namespace sptk {

FilterCoefficientsToSpectrum::FilterCoefficientsToSpectrum(
    int num_numerator_order, int num_denominator_order, int fft_length,
    SpectrumToSpectrum::InputOutputFormats output_format, double epsilon,
    double relative_floor_in_decibels)
    : num_numerator_order_(num_numerator_order),
      num_denominator_order_(num_denominator_order),
      fft_length_(fft_length),
      fast_fourier_transform_(fft_length_),
      spectrum_to_spectrum_(fft_length_, SpectrumToSpectrum::kPowerSpectrum,
                            output_format, epsilon, relative_floor_in_decibels),
      is_valid_(true) {
  if (num_numerator_order_ < 0 || num_denominator_order_ < 0 ||
      fft_length_ <= num_numerator_order_ ||
      fft_length_ <= num_denominator_order_ ||
      !fast_fourier_transform_.IsValid() || !spectrum_to_spectrum_.IsValid()) {
    is_valid_ = false;
  }
}

bool FilterCoefficientsToSpectrum::Run(
    const std::vector<double>& numerator_coefficients,
    const std::vector<double>& denominator_coefficients,
    std::vector<double>* spectrum,
    FilterCoefficientsToSpectrum::Buffer* buffer) const {
  // Check inputs.
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

  // Prepare memories.
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

  // Calculate numerators of transfer function.
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

  // Calculate denominators of transfer function.
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

  // Get gain of transfer function.
  const double gain(denominator_coefficients[0] * denominator_coefficients[0]);
  if (0.0 == gain) {
    return false;
  }

  // Calculate power spectrum.
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
        spectrum->begin(), [tmp](double y) { return tmp / y; });
  } else if (1 != numerator_length && 1 == denominator_length) {
    std::transform(
        buffer->numerator_of_transfer_function_.begin(),
        buffer->numerator_of_transfer_function_.begin() + output_length,
        spectrum->begin(), [gain](double x) { return gain * x; });
  } else {
    std::transform(
        buffer->numerator_of_transfer_function_.begin(),
        buffer->numerator_of_transfer_function_.begin() + output_length,
        buffer->denominator_of_transfer_function_.begin(), spectrum->begin(),
        [gain](double x, double y) { return gain * x / y; });
  }

  if (!spectrum_to_spectrum_.Run(spectrum)) {
    return false;
  }

  return true;
}

}  // namespace sptk
