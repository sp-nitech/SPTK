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

#include "SPTK/analysis/adaptive_mel_cepstral_analysis.h"

#include <algorithm>  // std::fill, std::transform
#include <cmath>      // std::log
#include <cstddef>    // std::size_t

namespace sptk {

AdaptiveMelCepstralAnalysis::AdaptiveMelCepstralAnalysis(
    int num_order, int num_pade_order, double alpha, double min_epsilon,
    double momentum, double forgetting_factor, double step_size_factor)
    : min_epsilon_(min_epsilon),
      momentum_(momentum),
      forgetting_factor_(forgetting_factor),
      step_size_factor_(step_size_factor),
      mlsa_digital_filter_(num_order, num_pade_order, alpha, false),
      mlsa_digital_filter_coefficients_to_mel_cepstrum_(num_order, alpha),
      is_valid_(true) {
  if (min_epsilon_ <= 0.0 || momentum_ < 0.0 || 1.0 <= momentum_ ||
      forgetting_factor_ < 0.0 || 1.0 <= forgetting_factor_ ||
      step_size_factor_ <= 0.0 || 1.0 <= step_size_factor_ ||
      !mlsa_digital_filter_.IsValid() ||
      !mlsa_digital_filter_coefficients_to_mel_cepstrum_.IsValid()) {
    is_valid_ = false;
    return;
  }
}

bool AdaptiveMelCepstralAnalysis::Run(
    double input_signal, double* prediction_error,
    std::vector<double>* mel_cepstrum,
    AdaptiveMelCepstralAnalysis::Buffer* buffer) const {
  // Check inputs.
  if (!is_valid_ || NULL == prediction_error || NULL == mel_cepstrum ||
      NULL == buffer) {
    return false;
  }

  // Prepare memories.
  const int num_order(GetNumOrder());
  if (buffer->mlsa_digital_filter_coefficients_.size() !=
      static_cast<std::size_t>(num_order + 1)) {
    buffer->mlsa_digital_filter_coefficients_.resize(num_order + 1);
    std::fill(buffer->mlsa_digital_filter_coefficients_.begin(),
              buffer->mlsa_digital_filter_coefficients_.end(), 0.0);
  }
  if (buffer->inverse_mlsa_digital_filter_coefficients_.size() !=
      static_cast<std::size_t>(num_order + 1)) {
    buffer->inverse_mlsa_digital_filter_coefficients_.resize(num_order + 1);
    buffer->inverse_mlsa_digital_filter_coefficients_[0] = 0.0;
  }
  if (buffer->buffer_for_phi_digital_filter_.size() !=
      static_cast<std::size_t>(num_order + 1)) {
    buffer->buffer_for_phi_digital_filter_.resize(num_order + 1);
    std::fill(buffer->buffer_for_phi_digital_filter_.begin(),
              buffer->buffer_for_phi_digital_filter_.end(), 0.0);
  }
  if (buffer->gradient_.size() != static_cast<std::size_t>(num_order)) {
    buffer->gradient_.resize(num_order);
    std::fill(buffer->gradient_.begin(), buffer->gradient_.end(), 0.0);
  }

  // Apply inverse MLSA digital filter.
  std::transform(buffer->mlsa_digital_filter_coefficients_.begin() + 1,
                 buffer->mlsa_digital_filter_coefficients_.end(),
                 buffer->inverse_mlsa_digital_filter_coefficients_.begin() + 1,
                 [](double x) { return -x; });
  double curr_prediction_error;
  if (!mlsa_digital_filter_.Run(
          buffer->inverse_mlsa_digital_filter_coefficients_, input_signal,
          &curr_prediction_error, &buffer->buffer_for_mlsa_digital_filter_)) {
    return false;
  }

  // Apply phi digital filter.
  {
    const double alpha(GetAlpha());
    const double beta(1.0 - alpha * alpha);
    double* e(&buffer->buffer_for_phi_digital_filter_[0]);
    e[0] = alpha * e[0] + beta * buffer->prev_prediction_error_;
    for (int i(1); i < num_order; ++i) {
      e[i] += alpha * (e[i + 1] - e[i - 1]);
    }
    for (int i(num_order); 0 < i; --i) {
      e[i] = e[i - 1];
    }
  }

  // Update epsilon using Eq. (29).
  double curr_epsilon((forgetting_factor_ * buffer->prev_epsilon_) +
                      (1.0 - forgetting_factor_) *
                          (curr_prediction_error * curr_prediction_error));
  if (curr_epsilon < min_epsilon_) {
    curr_epsilon = min_epsilon_;
  }

  // Update MLSA digital filter coefficients using Eq. (27).
  if (0 < num_order) {
    const double sigma(2.0 * (1.0 - momentum_) * curr_prediction_error);
    const double mu(step_size_factor_ / (num_order * curr_epsilon));
    const double* e(&buffer->buffer_for_phi_digital_filter_[1]);
    double* gradient(&buffer->gradient_[0]);
    double* b(&buffer->mlsa_digital_filter_coefficients_[1]);
    for (int i(0); i < num_order; ++i) {
      gradient[i] = momentum_ * gradient[i] - sigma * e[i];
      b[i] -= mu * gradient[i];
    }
  }
  buffer->mlsa_digital_filter_coefficients_[0] = 0.5 * std::log(curr_epsilon);

  // Store outputs.
  *prediction_error = curr_prediction_error;
  buffer->prev_prediction_error_ = curr_prediction_error;
  buffer->prev_epsilon_ = curr_epsilon;

  if (!mlsa_digital_filter_coefficients_to_mel_cepstrum_.Run(
          buffer->mlsa_digital_filter_coefficients_, mel_cepstrum)) {
    return false;
  }

  return true;
}

}  // namespace sptk
