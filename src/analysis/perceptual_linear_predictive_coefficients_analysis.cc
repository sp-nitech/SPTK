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

#include "SPTK/analysis/perceptual_linear_predictive_coefficients_analysis.h"

#include <algorithm>  // std::copy, std::reverse_copy, std::transform
#include <cmath>      // std::exp, std::pow, std::sin
#include <cstddef>    // std::size_t
#include <vector>     // std::vector

namespace sptk {

PerceptualLinearPredictiveCoefficientsAnalysis::
    PerceptualLinearPredictiveCoefficientsAnalysis(
        int fft_length, int num_channel, int num_order,
        int liftering_coefficient, double compression_factor,
        double sampling_rate, double lowest_frequency, double highest_frequency,
        double floor)
    : liftering_coefficient_(liftering_coefficient),
      compression_factor_(compression_factor),
      mel_filter_bank_analysis_(fft_length, num_channel, sampling_rate,
                                lowest_frequency, highest_frequency, floor,
                                true),
      inverse_fourier_transform_(2 * GetNumChannel() + 2),
      levinson_durbin_recursion_(num_order),
      linear_predictive_coefficients_to_cepstrum_(num_order, num_order),
      is_valid_(true) {
  if (num_channel <= num_order || liftering_coefficient_ <= 0 ||
      compression_factor_ <= 0.0 || !mel_filter_bank_analysis_.IsValid() ||
      !inverse_fourier_transform_.IsValid() ||
      !levinson_durbin_recursion_.IsValid() ||
      !linear_predictive_coefficients_to_cepstrum_.IsValid()) {
    is_valid_ = false;
    return;
  }

  std::vector<double> center_frequencies;
  if (!mel_filter_bank_analysis_.GetCenterFrequencies(&center_frequencies)) {
    is_valid_ = false;
    return;
  }

  equal_loudness_curve_.resize(num_channel);
  double* e(&(equal_loudness_curve_[0]));
  for (int m(0); m < num_channel; ++m) {
    const double f1(center_frequencies[m] * center_frequencies[m]);
    const double f2(f1 / (f1 + 1.6e5));
    e[m] = f2 * f2 * (f1 + 1.44e6) / (f1 + 9.61e6);
  }

  cepstal_weights_.resize(num_order);
  double* w(&(cepstal_weights_[0]));
  for (int m(0); m < num_order; ++m) {
    const double theta(sptk::kPi * (m + 1) / liftering_coefficient_);
    w[m] = 1.0 + 0.5 * liftering_coefficient_ * std::sin(theta);
  }
}

bool PerceptualLinearPredictiveCoefficientsAnalysis::Run(
    const std::vector<double>& power_spectrum, std::vector<double>* plp,
    double* energy,
    PerceptualLinearPredictiveCoefficientsAnalysis::Buffer* buffer) const {
  // Check inputs.
  if (!is_valid_ || NULL == plp || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  const int num_order(GetNumOrder());
  const int num_channel(GetNumChannel());
  const int dft_length(inverse_fourier_transform_.GetLength());
  if (plp->size() != static_cast<std::size_t>(num_order + 1)) {
    plp->resize(num_order + 1);
  }
  if (buffer->spectrum_.size() != static_cast<std::size_t>(num_channel)) {
    buffer->spectrum_.resize(num_channel);
  }
  if (buffer->real_part_input_.size() != static_cast<std::size_t>(dft_length)) {
    buffer->real_part_input_.resize(dft_length);
  }
  if (buffer->imag_part_input_.size() != static_cast<std::size_t>(dft_length)) {
    buffer->imag_part_input_.resize(dft_length);
  }

  if (!mel_filter_bank_analysis_.Run(power_spectrum,
                                     &buffer->filter_bank_output_, energy)) {
    return false;
  }

  std::transform(buffer->filter_bank_output_.begin(),
                 buffer->filter_bank_output_.end(),
                 equal_loudness_curve_.begin(), buffer->spectrum_.begin(),
                 [this](double x, double e) {
                   return std::pow(std::exp(x) * e, compression_factor_);
                 });

  buffer->real_part_input_[0] = buffer->spectrum_[0];
  std::copy(buffer->spectrum_.begin(), buffer->spectrum_.end(),
            buffer->real_part_input_.begin() + 1);
  buffer->real_part_input_[num_channel + 1] =
      buffer->spectrum_[num_channel - 1];
  std::reverse_copy(buffer->spectrum_.begin(), buffer->spectrum_.end(),
                    buffer->real_part_input_.begin() + num_channel + 2);

  // Convert spectrum to autocorrelation.
  if (!inverse_fourier_transform_.Run(
          buffer->real_part_input_, buffer->imag_part_input_,
          &buffer->real_part_output_, &buffer->imag_part_output_)) {
    return false;
  }

  // Convert autocorrelation to LPC.
  bool is_stable;
  buffer->real_part_output_.resize(num_order + 1);
  if (!levinson_durbin_recursion_.Run(
          &buffer->real_part_output_, &is_stable,
          &buffer->buffer_for_levinson_durbin_recursion_)) {
    return false;
  }

  // Convert LPC to cepstrum.
  if (!linear_predictive_coefficients_to_cepstrum_.Run(
          buffer->real_part_output_, &buffer->cepstrum_)) {
    return false;
  }

  (*plp)[0] = buffer->cepstrum_[0] * 2.0;

  // Lifter.
  std::transform(buffer->cepstrum_.begin() + 1,
                 buffer->cepstrum_.begin() + 1 + num_order,
                 cepstal_weights_.begin(), plp->begin() + 1,
                 [](double c, double w) { return c * w; });

  return true;
}

}  // namespace sptk
