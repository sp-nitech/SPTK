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

#include "SPTK/analysis/mel_cepstral_analysis.h"

#include <algorithm>   // std::copy, std::fill, std::transform, etc.
#include <cfloat>      // DBL_MAX
#include <cmath>       // std::exp, std::fabs, std::log
#include <cstddef>     // std::size_t
#include <functional>  // std::minus, std::plus

namespace {

void CoefficientsFrequencyTransform(const std::vector<double>& input,
                                    int input_length, int output_length,
                                    double alpha, std::vector<double>* output,
                                    std::vector<double>* buffer) {
  if (output->size() != static_cast<std::size_t>(output_length)) {
    output->resize(output_length);
  }

  if (0.0 == alpha) {
    if (input_length < output_length) {
      std::copy(input.begin(), input.end(), output->begin());
      std::fill(output->begin() + input_length, output->end(), 0.0);
    } else {
      std::copy(input.begin(), input.begin() + output_length, output->begin());
    }
    return;
  }

  if (buffer->size() != static_cast<std::size_t>(output_length)) {
    buffer->resize(output_length);
  }

  const double* c(&(input[0]));
  double* d(&((*buffer)[0]));
  double* g(&((*output)[0]));

  std::fill(g, g + output_length, 0.0);

  for (int i(input_length - 1); 0 <= i; --i) {
    d[0] = g[0];
    g[0] = c[i];
    for (int j(1); j < output_length; ++j) {
      d[j] = g[j];
      g[j] = d[j - 1] + alpha * (d[j] - g[j - 1]);
    }
  }
}

}  // namespace

namespace sptk {

MelCepstralAnalysis::MelCepstralAnalysis(int fft_length, int num_order,
                                         double alpha, int num_iteration,
                                         double convergence_threshold)
    : fft_length_(fft_length),
      num_order_(num_order),
      alpha_(alpha),
      num_iteration_(num_iteration),
      convergence_threshold_(convergence_threshold),
      fourier_transform_(fft_length_),
      inverse_fourier_transform_(fft_length_),
      frequency_transform_(fft_length_ / 2, num_order_, alpha_),
      inverse_frequency_transform_(num_order_, fft_length_ / 2, -alpha_),
      toeplitz_plus_hankel_system_solver_(num_order_, true),
      is_valid_(true) {
  if (num_iteration_ < 0 || convergence_threshold_ < 0.0 ||
      !fourier_transform_.IsValid() || !inverse_fourier_transform_.IsValid() ||
      !frequency_transform_.IsValid() ||
      !inverse_frequency_transform_.IsValid() ||
      !toeplitz_plus_hankel_system_solver_.IsValid()) {
    is_valid_ = false;
    return;
  }

  // Compute (-a)^0, (-a)^1, (-a)^2, ..., (-a)^M.
  alpha_vector_.resize(num_order_ + 1);
  alpha_vector_[0] = 1.0;
  for (int m(1); m <= num_order_; ++m) {
    alpha_vector_[m] = -alpha_ * alpha_vector_[m - 1];
  }
}

bool MelCepstralAnalysis::Run(const std::vector<double>& periodogram,
                              std::vector<double>* mel_cepstrum,
                              MelCepstralAnalysis::Buffer* buffer) const {
  // Check inputs.
  const int half_fft_length(fft_length_ / 2);
  if (!is_valid_ ||
      periodogram.size() != static_cast<std::size_t>(half_fft_length + 1) ||
      NULL == mel_cepstrum || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  const int length(num_order_ + 1);
  if (mel_cepstrum->size() != static_cast<std::size_t>(length)) {
    mel_cepstrum->resize(length);
  }
  if (buffer->log_periodogram_.size() !=
      static_cast<std::size_t>(half_fft_length + 1)) {
    buffer->log_periodogram_.resize(half_fft_length + 1);
  }
  if (buffer->cepstrum_.size() != static_cast<std::size_t>(fft_length_)) {
    buffer->cepstrum_.resize(fft_length_);
  }
  if (buffer->rt_.size() != static_cast<std::size_t>(2 * length - 1)) {
    buffer->rt_.resize(2 * length - 1);
  }
  if (buffer->rr_.size() != static_cast<std::size_t>(2 * length - 1)) {
    buffer->rr_.resize(2 * length - 1);
  }
  if (buffer->ra_.size() != static_cast<std::size_t>(length)) {
    buffer->ra_.resize(length);
  }
  if (buffer->gradient_.size() != static_cast<std::size_t>(length)) {
    buffer->gradient_.resize(length);
  }

  // Make an initial guess.
  {
    std::transform(periodogram.begin(), periodogram.end(),
                   buffer->log_periodogram_.begin(),
                   [](double p) { return std::log(p); });

    // \log I_N -> c
    std::copy(buffer->log_periodogram_.begin(), buffer->log_periodogram_.end(),
              buffer->cepstrum_.begin());
    std::reverse_copy(buffer->log_periodogram_.begin() + 1,
                      buffer->log_periodogram_.end() - 1,
                      buffer->cepstrum_.begin() + half_fft_length + 1);
    if (!inverse_fourier_transform_.Run(
            buffer->cepstrum_, &buffer->cepstrum_, &buffer->imag_part_output_,
            &buffer->buffer_for_inverse_fourier_transform_)) {
      return false;
    }
    buffer->cepstrum_[0] *= 0.5;
    buffer->cepstrum_[half_fft_length] *= 0.5;

    // c -> \tilde{c}
    buffer->cepstrum_.resize(half_fft_length + 1);
    if (!frequency_transform_.Run(buffer->cepstrum_, mel_cepstrum,
                                  &buffer->buffer_for_frequency_transform_)) {
      return false;
    }
  }

  // Perform Newton-Raphson method.
  double prev_epsilon(DBL_MAX);
  for (int n(0); n < num_iteration_; ++n) {
    // \tilde{c} -> c
    buffer->cepstrum_.resize(half_fft_length + 1);
    if (!inverse_frequency_transform_.Run(
            *mel_cepstrum, &buffer->cepstrum_,
            &buffer->buffer_for_inverse_frequency_transform_)) {
      return false;
    }

    // c -> \log D
    buffer->cepstrum_.resize(fft_length_, 0.0);
    if (!fourier_transform_.Run(buffer->cepstrum_, &buffer->d_,
                                &buffer->imag_part_output_,
                                &buffer->buffer_for_fourier_transform_)) {
      return false;
    }

    // \log D -> I_N / |D|^2
    std::transform(buffer->log_periodogram_.begin(),
                   buffer->log_periodogram_.end(), buffer->d_.begin(),
                   buffer->d_.begin(),
                   [](double x, double d) { return std::exp(x - d - d); });
    std::reverse_copy(buffer->d_.begin() + 1,
                      buffer->d_.begin() + half_fft_length,
                      buffer->d_.begin() + half_fft_length + 1);

    // I_N / |D|^2 -> r
    if (!inverse_fourier_transform_.Run(
            buffer->d_, &buffer->r_, &buffer->imag_part_output_,
            &buffer->buffer_for_inverse_fourier_transform_)) {
      return false;
    }

    // r -> \tilde{r}
    CoefficientsFrequencyTransform(buffer->r_, half_fft_length + 1,
                                   2 * length - 1, alpha_, &buffer->rt_,
                                   &buffer->b_);

    // Check convergence.
    // (Note that the check can be done after updating mel-cepstrum, but to
    // retain the compatibility with SPTK3, this block is written here)
    {
      const double epsilon(buffer->rt_[0]);
      const double relative_change((epsilon - prev_epsilon) / epsilon);
      if (std::fabs(relative_change) < convergence_threshold_) {
        break;
      }
      prev_epsilon = epsilon;
    }

    // \tilde{r} -> R
    std::reverse_copy(buffer->rt_.begin() + 1, buffer->rt_.begin() + length,
                      buffer->rr_.begin());
    std::copy(buffer->rt_.begin(), buffer->rt_.begin() + length,
              buffer->rr_.begin() + length - 1);

    // \tilde{r} -> \tilde{r} - a
    std::transform(buffer->rt_.begin(), buffer->rt_.begin() + length,
                   alpha_vector_.begin(), buffer->ra_.begin(),
                   std::minus<double>());

    // Compute \Delta \tilde{c}.
    if (!toeplitz_plus_hankel_system_solver_.Run(
            buffer->rr_, buffer->rt_, buffer->ra_, &buffer->gradient_,
            &buffer->buffer_for_system_solver_)) {
      return false;
    }

    // Update \tilde{c}.
    std::transform(mel_cepstrum->begin(), mel_cepstrum->end(),
                   buffer->gradient_.begin(), mel_cepstrum->begin(),
                   std::plus<double>());
  }

  return true;
}

}  // namespace sptk
