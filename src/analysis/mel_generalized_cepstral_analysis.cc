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

#include "SPTK/analysis/mel_generalized_cepstral_analysis.h"

#include <algorithm>   // std::copy, std::fill, std::transform, etc.
#include <cmath>       // std::exp, std::fabs, std::pow, std::sqrt
#include <cstddef>     // std::size_t
#include <functional>  // std::plus

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

  if (buffer->size() < static_cast<std::size_t>(output_length)) {
    buffer->resize(output_length);
  }

  const double beta(1.0 - alpha * alpha);
  const double* b(&(input[0]));
  double* d(&((*buffer)[0]));
  double* g(&((*output)[0]));

  std::fill(g, g + output_length, 0.0);

  for (int i(input_length - 1); 0 <= i; --i) {
    d[0] = g[0];
    g[0] = b[i];
    if (1 < output_length) {
      d[1] = g[1];
      g[1] = beta * d[0] + alpha * d[1];
    }
    for (int j(2); j < output_length; ++j) {
      d[j] = g[j];
      g[j] = d[j - 1] + alpha * (d[j] - g[j - 1]);
    }
  }
}

// Applying CoefficientsFrequencyTransform and PTransform is equivalent to
// Eq. (41).
void PTransform(double alpha, std::vector<double>* p) {
  if (0.0 == alpha) return;

  const int order(static_cast<int>(p->size()) - 1);
  double* x(&((*p)[0]));
  double d(x[order]);
  for (int k(order); 0 < k; --k) {
    const double o(x[k] + alpha * d);
    d = x[k];
    x[k] = o;
  }
  const double o(alpha * d);
  x[0] = (1.0 - alpha * alpha) * x[0] + o + o;
}

// Applying CoefficientsFrequencyTransform and QTransform is equivalent to
// Eq. (42).
void QTransform(double alpha, std::vector<double>* q) {
  if (0.0 == alpha) return;

  const int order(static_cast<int>(q->size()) - 1);
  double* x(&((*q)[0]));
  double d(x[1]);
  for (int k(1); k <= order; ++k) {
    const double o(x[k] + alpha * d);
    d = x[k];
    x[k] = o;
  }
}

}  // namespace

namespace sptk {

MelGeneralizedCepstralAnalysis::MelGeneralizedCepstralAnalysis(
    int fft_length, int num_order, double alpha, double gamma,
    int num_iteration, double convergence_threshold)
    : fft_length_(fft_length),
      num_order_(num_order),
      alpha_(alpha),
      gamma_(gamma),
      num_iteration_(num_iteration),
      convergence_threshold_(convergence_threshold),
      fourier_transform_(fft_length_),
      inverse_fourier_transform_(fft_length_),
      complex_valued_inverse_fourier_transform_(fft_length_),
      toeplitz_plus_hankel_system_solver_(num_order_ - 1, true),
      generalized_cepstrum_gain_normalization_(num_order_, gamma_),
      generalized_cepstrum_inverse_gain_normalization_gamma_minus_one_(
          num_order_, -1.0),
      generalized_cepstrum_inverse_gain_normalization_(num_order_, gamma_),
      mel_cepstrum_to_mlsa_digital_filter_coefficients_(num_order_, alpha_),
      mlsa_digital_filter_coefficients_to_mel_cepstrum_(num_order_, alpha_),
      mel_generalized_cepstrum_transform_(num_order_, 0.0, -1.0, false, false,
                                          num_order_, 0.0, gamma_, false,
                                          false),
      mel_cepstral_analysis_(NULL),
      is_valid_(true) {
  if (!sptk::IsValidGamma(gamma_) || 0.0 < gamma_ || num_iteration_ < 0 ||
      convergence_threshold_ < 0.0 || !fourier_transform_.IsValid() ||
      !inverse_fourier_transform_.IsValid() ||
      !complex_valued_inverse_fourier_transform_.IsValid() ||
      !toeplitz_plus_hankel_system_solver_.IsValid() ||
      !generalized_cepstrum_gain_normalization_.IsValid() ||
      !generalized_cepstrum_inverse_gain_normalization_.IsValid() ||
      !mel_cepstrum_to_mlsa_digital_filter_coefficients_.IsValid() ||
      !mlsa_digital_filter_coefficients_to_mel_cepstrum_.IsValid() ||
      !mel_generalized_cepstrum_transform_.IsValid()) {
    is_valid_ = false;
    return;
  }

  if (0.0 == gamma_) {
    mel_cepstral_analysis_ = new MelCepstralAnalysis(
        fft_length_, num_order_, alpha_, num_iteration_, convergence_threshold);
  }
}

bool MelGeneralizedCepstralAnalysis::Run(
    const std::vector<double>& periodogram,
    std::vector<double>* mel_generalized_cepstrum,
    MelGeneralizedCepstralAnalysis::Buffer* buffer) const {
  if (0.0 == gamma_) {
    return mel_cepstral_analysis_->Run(
        periodogram, mel_generalized_cepstrum,
        &buffer->buffer_for_mel_cepstral_analysis_);
  }

  // Check inputs.
  const int half_fft_length(fft_length_ / 2);
  if (!is_valid_ ||
      periodogram.size() != static_cast<std::size_t>(half_fft_length + 1) ||
      NULL == mel_generalized_cepstrum || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  const int length(num_order_ + 1);
  if (mel_generalized_cepstrum->size() != static_cast<std::size_t>(length)) {
    mel_generalized_cepstrum->resize(length);
  }
  if (buffer->b_.size() != static_cast<std::size_t>(length)) {
    buffer->b_.resize(length);
  }
  if (buffer->p_real_.size() != static_cast<std::size_t>(fft_length_)) {
    buffer->p_real_.resize(fft_length_);
  }
  if (buffer->p_trim_.size() != static_cast<std::size_t>(2 * num_order_ - 1)) {
    buffer->p_trim_.resize(2 * num_order_ - 1);
  }
  if (buffer->q_real_.size() != static_cast<std::size_t>(fft_length_)) {
    buffer->q_real_.resize(fft_length_);
  }
  if (buffer->q_imag_.size() != static_cast<std::size_t>(fft_length_)) {
    buffer->q_imag_.resize(fft_length_);
  }
  if (buffer->q_.size() != static_cast<std::size_t>(2 * length - 1)) {
    buffer->q_.resize(2 * length - 1);
  }
  if (buffer->q_trim_.size() != static_cast<std::size_t>(2 * num_order_ - 1)) {
    buffer->q_trim_.resize(2 * num_order_ - 1);
  }
  if (buffer->r_real_.size() != static_cast<std::size_t>(fft_length_)) {
    buffer->r_real_.resize(fft_length_);
  }
  if (buffer->r_imag_.size() != static_cast<std::size_t>(fft_length_)) {
    buffer->r_imag_.resize(fft_length_);
  }
  if (buffer->r_.size() != static_cast<std::size_t>(length)) {
    buffer->r_.resize(length);
  }
  if (buffer->r_trim_.size() != static_cast<std::size_t>(num_order_)) {
    buffer->r_trim_.resize(num_order_);
  }
  if (buffer->periodogram_.size() != static_cast<std::size_t>(fft_length_)) {
    buffer->periodogram_.resize(fft_length_);
  }

  // Make full periodogram.
  std::copy(periodogram.begin(), periodogram.end(),
            buffer->periodogram_.begin());
  std::reverse_copy(periodogram.begin() + 1, periodogram.end() - 1,
                    buffer->periodogram_.begin() + half_fft_length + 1);

  // Make an initial guess.
  double prev_epsilon;
  std::fill(buffer->b_.begin(), buffer->b_.end(), 0.0);
  if (!NewtonRaphsonMethod(-1.0, &prev_epsilon, buffer)) {
    return false;
  }
  if (-1.0 != gamma_) {
    // K, b'r -> br
    if (!generalized_cepstrum_inverse_gain_normalization_gamma_minus_one_.Run(
            &buffer->b_)) {
      return false;
    }
    // br -> cr
    if (!mlsa_digital_filter_coefficients_to_mel_cepstrum_.Run(buffer->b_,
                                                               &buffer->c_)) {
      return false;
    }
    // cr -> cr'
    if (!mel_generalized_cepstrum_transform_.Run(
            buffer->c_, &buffer->c_,
            &buffer->buffer_for_mel_generalized_cepstrum_transform_)) {
      return false;
    }
    // cr' -> br'
    if (!mel_cepstrum_to_mlsa_digital_filter_coefficients_.Run(buffer->c_,
                                                               &buffer->b_)) {
      return false;
    }
    // br' -> K, b'r'
    if (!generalized_cepstrum_gain_normalization_.Run(&buffer->b_)) {
      return false;
    }
  }

  // Update coefficients using gradient method.
  if (-1.0 != gamma_) {
    for (int n(1); n <= num_iteration_; ++n) {
      double epsilon;
      if (!NewtonRaphsonMethod(gamma_, &epsilon, buffer)) {
        return false;
      }

      // Check convergence.
      const double relative_change((epsilon - prev_epsilon) / epsilon);
      if (std::fabs(relative_change) < convergence_threshold_) {
        break;
      }
      prev_epsilon = epsilon;
    }
  }

  // K, b'r' -> br
  if (!generalized_cepstrum_inverse_gain_normalization_.Run(&buffer->b_)) {
    return false;
  }
  // br -> cr
  if (!mlsa_digital_filter_coefficients_to_mel_cepstrum_.Run(
          buffer->b_, mel_generalized_cepstrum)) {
    return false;
  }

  return true;
}

bool MelGeneralizedCepstralAnalysis::NewtonRaphsonMethod(
    double gamma, double* epsilon,
    MelGeneralizedCepstralAnalysis::Buffer* buffer) const {
  const int length(num_order_ + 1);

  // Calculate Eq. (44).
  buffer->b_[0] = 0.0;
  CoefficientsFrequencyTransform(buffer->b_, length, fft_length_, -alpha_,
                                 &buffer->c_,
                                 &buffer->buffer_for_frequency_transform_);

  // Calculate a part of Eq. (45).
  if (!fourier_transform_.Run(buffer->c_, &buffer->real_, &buffer->imag_,
                              &buffer->buffer_for_fourier_transform_)) {
    return false;
  }

  if (-1.0 == gamma) {
    std::copy(buffer->periodogram_.begin(), buffer->periodogram_.end(),
              buffer->p_real_.begin());
  } else if (0.0 == gamma) {
    std::transform(buffer->periodogram_.begin(), buffer->periodogram_.end(),
                   buffer->real_.begin(), buffer->p_real_.begin(),
                   [](double x, double d) { return x / std::exp(d + d); });
  } else {
    const double* pg(&buffer->periodogram_[0]);
    double* c_re(&buffer->real_[0]);
    double* c_im(&buffer->imag_[0]);
    double* p_re(&buffer->p_real_[0]);
    double* q_re(&buffer->q_real_[0]);
    double* q_im(&buffer->q_imag_[0]);
    double* r_re(&buffer->r_real_[0]);
    double* r_im(&buffer->r_imag_[0]);
    for (int i(0); i < fft_length_; ++i) {
      // D^\gamma
      const double x(1.0 + gamma * c_re[i]);
      const double y(gamma * c_im[i]);

      // |D^\gamma|^2
      const double xx(x * x);
      const double yy(y * y);
      const double d_gamma(xx + yy);

      // 1 / |D|^2
      const double d_inv(std::pow(d_gamma, -1.0 / gamma));

      // I_N / |D^{1+\gamma}|^2
      const double p(pg[i] * d_inv / d_gamma);
      p_re[i] = p;

      // I_N / |D^{1+2\gamma}|^2 * D^{2\gamma}
      const double q(p / d_gamma);
      q_re[i] = q * (xx - yy);
      q_im[i] = q * (2.0 * x * y);

      // I_N / |D^{1+\gamma}|^2 * D^\gamma
      r_re[i] = p * x;
      r_im[i] = p * y;
    }
  }

  // Calculate Eq. (38).
  if (!inverse_fourier_transform_.Run(
          buffer->p_real_, &buffer->real_, &buffer->imag_,
          &buffer->buffer_for_inverse_fourier_transform_)) {
    return false;
  }

  // Obtain p~' (warped version of p using Eq. (43)).
  CoefficientsFrequencyTransform(buffer->real_, fft_length_, 2 * length - 1,
                                 alpha_, &buffer->p_,
                                 &buffer->buffer_for_frequency_transform_);

  if (0.0 == gamma || -1.0 == gamma) {
    std::copy(buffer->p_.begin(), buffer->p_.end(), buffer->q_.begin());
    std::copy(buffer->p_.begin(), buffer->p_.begin() + length,
              buffer->r_.begin());
  } else {
    // Calculate Eq. (39).
    if (!complex_valued_inverse_fourier_transform_.Run(
            buffer->q_real_, buffer->q_imag_, &buffer->q_real_,
            &buffer->q_imag_)) {
      return false;
    }

    // Calculate Eq. (40).
    if (!complex_valued_inverse_fourier_transform_.Run(
            buffer->r_real_, buffer->r_imag_, &buffer->r_real_,
            &buffer->r_imag_)) {
      return false;
    }

    // Obtain q~' (warped version of q using Eq. (43)).
    CoefficientsFrequencyTransform(buffer->q_real_, fft_length_, 2 * length - 1,
                                   alpha_, &buffer->q_,
                                   &buffer->buffer_for_frequency_transform_);

    // Calculate Eq. (43).
    CoefficientsFrequencyTransform(buffer->r_real_, fft_length_, length, alpha_,
                                   &buffer->r_,
                                   &buffer->buffer_for_frequency_transform_);
  }

  // Transform p~' to p~.
  PTransform(alpha_, &buffer->p_);

  // Transform q~' to q~.
  QTransform(alpha_, &buffer->q_);

  // Make p~(M-1), ..., p~(0), ..., p~(M-1). (Eq. (30))
  std::reverse_copy(buffer->p_.begin() + 1, buffer->p_.begin() + num_order_,
                    buffer->p_trim_.begin());
  std::copy(buffer->p_.begin(), buffer->p_.begin() + num_order_,
            buffer->p_trim_.begin() + num_order_ - 1);

  // Extract q~(2), ..., q~(2M). (Eq. (31))
  std::copy(buffer->q_.begin() + 2, buffer->q_.end(), buffer->q_trim_.begin());

  // Extract r~(1), ..., r~(M). (Eq. (26))
  std::copy(buffer->r_.begin() + 1, buffer->r_.end(), buffer->r_trim_.begin());

  // Calculate epsilon using Eq. (36).
  if (0.0 == gamma) {
    *epsilon = buffer->r_[0];
  } else if (-1.0 != gamma) {
    double sum(0.0);
    double* r(&buffer->r_[0]);
    double* b(&buffer->b_[0]);
    for (int m(1); m <= num_order_; ++m) {
      sum += r[m] * b[m];
    }
    *epsilon = r[0] + gamma * sum;
  }

  // Calculate (1 + \gamma) Q.
  if (0.0 != gamma) {
    const double g(1.0 + gamma);
    std::transform(buffer->q_trim_.begin(), buffer->q_trim_.end(),
                   buffer->q_trim_.begin(), [g](double x) { return g * x; });
  }

  // Solve Eq. (29).
  if (!toeplitz_plus_hankel_system_solver_.Run(
          buffer->p_trim_, buffer->q_trim_, buffer->r_trim_, &buffer->gradient_,
          &buffer->buffer_for_system_solver_)) {
    return false;
  }

  // Update b' using Eq. (35).
  std::transform(buffer->b_.begin() + 1, buffer->b_.end(),
                 buffer->gradient_.begin(), buffer->b_.begin() + 1,
                 std::plus<double>());

  // Since iteration is not required when \gamma=-1, update gain using
  // updated b'.
  if (-1.0 == gamma) {
    double sum(0.0);
    double* r(&buffer->r_[0]);
    double* b(&buffer->b_[0]);
    for (int m(1); m <= num_order_; ++m) {
      sum += r[m] * b[m];
    }
    *epsilon = r[0] + gamma * sum;
  }

  // Update gain using Eq. (25).
  const double gain(std::sqrt(*epsilon));
  buffer->b_[0] = gain;

  return true;
}

}  // namespace sptk
