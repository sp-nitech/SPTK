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

#include "SPTK/analysis/second_order_all_pass_mel_cepstral_analysis.h"

#include <algorithm>   // std::copy, std::fill, std::transform, etc.
#include <cfloat>      // DBL_MAX
#include <cmath>       // std::cos, std::exp, std::fabs, std::log, std::sin
#include <cstddef>     // std::size_t
#include <functional>  // std::minus, std::plus
#include <numeric>     // std::inner_product

#include "SPTK/math/inverse_fast_fourier_transform.h"

namespace {

bool CoefficientsFrequencyTransform(const std::vector<double>& input,
                                    int input_length, int output_length,
                                    int fft_length, double alpha, double theta,
                                    std::vector<double>* output,
                                    std::vector<std::vector<double> >* buffer) {
  if (output->size() != static_cast<std::size_t>(output_length)) {
    output->resize(output_length);
  }

  if (0.0 == alpha && 0.0 == theta) {
    if (input_length < output_length) {
      std::copy(input.begin(), input.end(), output->begin());
      std::fill(output->begin() + input_length, output->end(), 0.0);
    } else {
      std::copy(input.begin(), input.begin() + output_length, output->begin());
    }
    return true;
  }

  if (buffer->size() != static_cast<std::size_t>(output_length)) {
    std::vector<std::vector<double> > real(output_length,
                                           std::vector<double>(fft_length));
    std::vector<std::vector<double> > imag(output_length,
                                           std::vector<double>(fft_length));
    {
      std::vector<double> ww(fft_length);
      {
        const double delta(sptk::kTwoPi / fft_length);
        for (int j(0); j < fft_length; ++j) {
          ww[j] = sptk::Warp(delta * j, alpha, theta);
        }
      }

      sptk::InverseFastFourierTransform inverse_fourier_transform(fft_length);
      for (int m(0); m < output_length; ++m) {
        for (int j(0); j < fft_length; ++j) {
          real[m][j] = std::cos(ww[j] * m);
          imag[m][j] = -std::sin(ww[j] * m);
        }

        if (!inverse_fourier_transform.Run(&real[m], &imag[m])) {
          return false;
        }

        for (int j(1); j < input_length; ++j) {
          real[m][j] += real[m][fft_length - j];
        }
      }
    }

    buffer->resize(output_length);
    for (int m2(0); m2 < output_length; ++m2) {
      (*buffer)[m2].resize(input_length);
      for (int m1(0); m1 < input_length; ++m1) {
        (*buffer)[m2][m1] = real[m2][m1];
      }
    }
  }

  for (int m2(0); m2 < output_length; ++m2) {
    (*output)[m2] = std::inner_product((*buffer)[m2].begin(),
                                       (*buffer)[m2].end(), input.begin(), 0.0);
  }

  return true;
}

}  // namespace

namespace sptk {

SecondOrderAllPassMelCepstralAnalysis::SecondOrderAllPassMelCepstralAnalysis(
    int fft_length, int num_order, int accuracy_factor, double alpha,
    double theta, int num_iteration, double convergence_threshold)
    : fft_length_(fft_length),
      num_order_(num_order),
      accuracy_factor_(accuracy_factor),
      alpha_(alpha),
      theta_(theta),
      num_iteration_(num_iteration),
      convergence_threshold_(convergence_threshold),
      fourier_transform_(fft_length_),
      inverse_fourier_transform_(fft_length_),
      frequency_transform_(fft_length_ / 2, num_order_,
                           fft_length_ * accuracy_factor, alpha_, theta_),
      inverse_frequency_transform_(num_order_, fft_length_ / 2,
                                   fft_length * accuracy_factor, alpha_,
                                   theta_),
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

  std::vector<double> seed({1.0});
  std::vector<std::vector<double> > buffer;
  if (!CoefficientsFrequencyTransform(seed, 1, num_order_ + 1,
                                      fft_length_ * accuracy_factor_, alpha_,
                                      theta_, &alpha_vector_, &buffer)) {
    is_valid_ = false;
    return;
  }
}

bool SecondOrderAllPassMelCepstralAnalysis::Run(
    const std::vector<double>& periodogram, std::vector<double>* mel_cepstrum,
    SecondOrderAllPassMelCepstralAnalysis::Buffer* buffer) const {
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
    if (!frequency_transform_.Run(buffer->cepstrum_, mel_cepstrum)) {
      return false;
    }
  }

  // Perform Newton-Raphson method.
  double prev_epsilon(DBL_MAX);
  for (int n(0); n < num_iteration_; ++n) {
    // \tilde{c} -> c
    buffer->cepstrum_.resize(half_fft_length + 1);
    if (!inverse_frequency_transform_.Run(*mel_cepstrum, &buffer->cepstrum_)) {
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
    if (!CoefficientsFrequencyTransform(buffer->r_, half_fft_length + 1,
                                        2 * length - 1,
                                        fft_length_ * accuracy_factor_, alpha_,
                                        theta_, &buffer->rt_, &buffer->b_)) {
      return false;
    }

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
