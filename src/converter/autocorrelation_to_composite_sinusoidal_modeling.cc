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

#include "SPTK/converter/autocorrelation_to_composite_sinusoidal_modeling.h"

#include <algorithm>  // std::copy, std::reverse, std::sort, std::transform
#include <cmath>      // std::acos, std::fabs, std::pow
#include <cstddef>    // std::size_t

namespace {

bool CalculateBinomialCoefficient(int n, int k, double* binomial_coefficient) {
  if (n < 0 || k < 0 || n < k || NULL == binomial_coefficient) {
    return false;
  }
  if (0 == k || n == k) {
    *binomial_coefficient = 1.0;
    return true;
  }

  std::vector<double> buffer(n);

  for (int i(0); i < n; ++i) {
    buffer[i] = 1.0;
    for (int j(i - 1); 0 < j; --j) {
      buffer[j] = buffer[j] + buffer[j - 1];
    }
  }
  buffer[n - 1] = 1.0;
  *binomial_coefficient = buffer[k] + buffer[k - 1];

  return true;
}

}  // namespace

namespace sptk {

AutocorrelationToCompositeSinusoidalModeling::
    AutocorrelationToCompositeSinusoidalModeling(int num_order,
                                                 int num_iteration,
                                                 double convergence_threshold)
    : num_order_(num_order),
      num_sine_wave_((num_order_ + 1) / 2),
      cholesky_solver_(num_sine_wave_ - 1),
      durand_kerner_method_(num_sine_wave_, num_iteration,
                            convergence_threshold),
      vandermonde_system_solver_(num_sine_wave_ - 1),
      is_valid_(true) {
  if (num_order_ < 0 || 0 == num_order_ % 2) {
    is_valid_ = false;
  }
}

bool AutocorrelationToCompositeSinusoidalModeling::Run(
    const std::vector<double>& autocorrelation,
    std::vector<double>* composite_sinusoidal_modeling,
    AutocorrelationToCompositeSinusoidalModeling::Buffer* buffer) const {
  // check inputs
  const int length(num_order_ + 1);
  if (!is_valid_ ||
      autocorrelation.size() != static_cast<std::size_t>(length) ||
      NULL == composite_sinusoidal_modeling || NULL == buffer) {
    return false;
  }

  // prepare memories
  if (composite_sinusoidal_modeling->size() !=
      static_cast<std::size_t>(length)) {
    composite_sinusoidal_modeling->resize(length);
  }

  // prepare buffer
  if (buffer->u_.size() != static_cast<std::size_t>(length)) {
    buffer->u_.resize(length);
  }
  if (buffer->u_first_half_.size() !=
      static_cast<std::size_t>(num_sine_wave_)) {
    buffer->u_first_half_.resize(num_sine_wave_);
  }
  if (buffer->u_second_half_.size() !=
      static_cast<std::size_t>(num_sine_wave_)) {
    buffer->u_second_half_.resize(num_sine_wave_);
  }
  if (buffer->u_symmetric_matrix_.GetNumDimension() != num_sine_wave_) {
    buffer->u_symmetric_matrix_.Resize(num_sine_wave_);
  }
  if (buffer->x_.size() != static_cast<std::size_t>(num_sine_wave_)) {
    buffer->x_.resize(num_sine_wave_);
  }
  if (buffer->x_real_part_.size() != static_cast<std::size_t>(num_sine_wave_)) {
    buffer->x_real_part_.resize(num_sine_wave_);
  }
  if (buffer->p_.size() != static_cast<std::size_t>(num_sine_wave_)) {
    buffer->p_.resize(num_sine_wave_);
  }
  if (buffer->frequencies_.size() != static_cast<std::size_t>(num_sine_wave_)) {
    buffer->frequencies_.resize(num_sine_wave_);
  }
  if (buffer->intensities_.size() != static_cast<std::size_t>(num_sine_wave_)) {
    buffer->intensities_.resize(num_sine_wave_);
  }

  // get value
  const double* input(&(autocorrelation[0]));
  const std::complex<double>* x(&(buffer->x_[0]));
  double* x_real_part(&(buffer->x_real_part_[0]));
  double* u(&(buffer->u_[0]));

  for (int l(0); l < length; ++l) {
    double sum(0.0);
    for (int k(0); k <= l; ++k) {
      double binomial_coefficient;
      if (!CalculateBinomialCoefficient(l, k, &binomial_coefficient)) {
        return false;
      }
      const int index((l < 2 * k) ? (2 * k - l) : (l - 2 * k));
      sum += binomial_coefficient * input[index];
    }
    u[l] = sum / std::pow(2.0, l);
  }

  for (int i(0); i < num_sine_wave_; ++i) {
    for (int j(i); j < num_sine_wave_; ++j) {
      buffer->u_symmetric_matrix_[i][j] = -u[i + j];
    }
  }

  std::copy(buffer->u_.begin() + num_sine_wave_, buffer->u_.end(),
            buffer->u_second_half_.begin());
  if (!cholesky_solver_.Run(buffer->u_symmetric_matrix_, buffer->u_second_half_,
                            &(buffer->p_),
                            &(buffer->cholesky_solver_buffer_))) {
    return false;
  }

  std::reverse(buffer->p_.begin(), buffer->p_.end());

  {
    bool is_converged;
    if (!durand_kerner_method_.Run(buffer->p_, &(buffer->x_), &is_converged)) {
      return false;
    }
    if (!is_converged) {
      return false;
    }
  }

  for (int i(0); i < num_sine_wave_; ++i) {
    x_real_part[i] = x[i].real();
    if (1.0 < std::fabs(x_real_part[i])) {
      return false;
    }
  }
  std::sort(buffer->x_real_part_.begin(), buffer->x_real_part_.end(),
            [](double x, double y) { return y < x; });

  std::transform(buffer->x_real_part_.begin(), buffer->x_real_part_.end(),
                 composite_sinusoidal_modeling->begin(),
                 [](double x) { return std::acos(x); });

  std::copy(buffer->u_.begin(), buffer->u_.begin() + num_sine_wave_,
            buffer->u_first_half_.begin());
  if (!vandermonde_system_solver_.Run(
          buffer->x_real_part_, buffer->u_first_half_, &(buffer->intensities_),
          &(buffer->vandermonde_system_solver_buffer_))) {
    return false;
  }
  std::copy(buffer->intensities_.begin(), buffer->intensities_.end(),
            composite_sinusoidal_modeling->begin() + num_sine_wave_);

  return true;
}

}  // namespace sptk
