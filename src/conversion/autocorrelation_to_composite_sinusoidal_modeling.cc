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
//                1996-2020  Nagoya Institute of Technology          //
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

#include "SPTK/conversion/autocorrelation_to_composite_sinusoidal_modeling.h"

#include <algorithm>  // std::copy, std::reverse, std::sort, std::transform
#include <cmath>      // std::acos, std::fabs, std::pow
#include <cstddef>    // std::size_t

namespace {

uint64_t CalculateBinomialCoefficient(int n, int k) {
  if (0 == k || n == k) {
    return 1.0;
  }

  std::vector<uint64_t> buffer(n);
  for (int i(0); i < n; ++i) {
    buffer[i] = 1;
    for (int j(i - 1); 0 < j; --j) {
      buffer[j] = buffer[j] + buffer[j - 1];
    }
  }
  buffer[n - 1] = 1;
  return buffer[k] + buffer[k - 1];
}

}  // namespace

namespace sptk {

AutocorrelationToCompositeSinusoidalModeling::
    AutocorrelationToCompositeSinusoidalModeling(int num_order,
                                                 int num_iteration,
                                                 double convergence_threshold)
    : num_order_(num_order),
      num_sine_wave_((num_order_ + 1) / 2),
      symmetric_system_solver_(num_sine_wave_ - 1),
      durand_kerner_method_(num_sine_wave_, num_iteration,
                            convergence_threshold),
      vandermonde_system_solver_(num_sine_wave_ - 1),
      is_valid_(true) {
  if (num_order_ < 0 || 0 == num_order_ % 2 ||
      !symmetric_system_solver_.IsValid() || !durand_kerner_method_.IsValid() ||
      !vandermonde_system_solver_.IsValid()) {
    is_valid_ = false;
    return;
  }
}

bool AutocorrelationToCompositeSinusoidalModeling::Run(
    const std::vector<double>& autocorrelation,
    std::vector<double>* composite_sinusoidal_modeling,
    AutocorrelationToCompositeSinusoidalModeling::Buffer* buffer) const {
  // Check inputs.
  const int length(num_order_ + 1);
  if (!is_valid_ ||
      autocorrelation.size() != static_cast<std::size_t>(length) ||
      NULL == composite_sinusoidal_modeling || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  const std::size_t size(static_cast<std::size_t>(num_sine_wave_));
  if (composite_sinusoidal_modeling->size() !=
      static_cast<std::size_t>(length)) {
    composite_sinusoidal_modeling->resize(length);
  }
  if (buffer->u_.size() != static_cast<std::size_t>(length)) {
    buffer->u_.resize(length);
  }
  if (buffer->u_first_half_.size() != size) {
    buffer->u_first_half_.resize(size);
  }
  if (buffer->u_second_half_.size() != size) {
    buffer->u_second_half_.resize(size);
  }
  if (buffer->u_symmetric_matrix_.GetNumDimension() != num_sine_wave_) {
    buffer->u_symmetric_matrix_.Resize(num_sine_wave_);
  }
  if (buffer->x_.size() != size) {
    buffer->x_.resize(size);
  }
  if (buffer->x_real_part_.size() != size) {
    buffer->x_real_part_.resize(size);
  }
  if (buffer->p_.size() != size) {
    buffer->p_.resize(size);
  }
  if (buffer->intensities_.size() != size) {
    buffer->intensities_.resize(size);
  }

  // Compute Hankel elements.
  {
    const double* input(&(autocorrelation[0]));
    double* u(&(buffer->u_[0]));

    for (int l(0); l < length; ++l) {
      long double sum(0.0);
      for (int k(0); k <= l; ++k) {
        const uint64_t binomial_coefficient(CalculateBinomialCoefficient(l, k));
        const int index((l < 2 * k) ? (2 * k - l) : (l - 2 * k));
        sum += (binomial_coefficient)*input[index];
      }
      u[l] = sum / std::pow(2.0, l);
    }

    for (int i(0); i < num_sine_wave_; ++i) {
      for (int j(i); j < num_sine_wave_; ++j) {
        buffer->u_symmetric_matrix_[i][j] = -u[i + j];
      }
    }
  }

  // Solve Hankel system.
  {
    std::copy(buffer->u_.begin() + num_sine_wave_, buffer->u_.end(),
              buffer->u_second_half_.begin());
    if (!symmetric_system_solver_.Run(
            buffer->u_symmetric_matrix_, buffer->u_second_half_, &buffer->p_,
            &buffer->symmetric_system_solver_buffer_)) {
      return false;
    }
  }

  // Compute roots.
  {
    std::reverse(buffer->p_.begin(), buffer->p_.end());
    {
      bool is_converged;
      if (!durand_kerner_method_.Run(buffer->p_, &(buffer->x_),
                                     &is_converged)) {
        return false;
      }
      if (!is_converged) {
        return false;
      }
    }

    double* x_real_part(&(buffer->x_real_part_[0]));
    {
      const std::complex<double>* x(&(buffer->x_[0]));
      for (int i(0); i < num_sine_wave_; ++i) {
        x_real_part[i] = x[i].real();
        if (1.0 < std::fabs(x_real_part[i])) {
          return false;
        }
      }
    }
  }

  // Compute Vander Monde elements.
  {
    std::sort(buffer->x_real_part_.begin(), buffer->x_real_part_.end(),
              [](double x, double y) { return y < x; });

    std::transform(buffer->x_real_part_.begin(), buffer->x_real_part_.end(),
                   composite_sinusoidal_modeling->begin(),
                   [](double x) { return std::acos(x); });
  }

  // Solve Vander Monde system.
  {
    std::copy(buffer->u_.begin(), buffer->u_.begin() + num_sine_wave_,
              buffer->u_first_half_.begin());
    if (!vandermonde_system_solver_.Run(
            buffer->x_real_part_, buffer->u_first_half_, &buffer->intensities_,
            &buffer->vandermonde_system_solver_buffer_)) {
      return false;
    }

    std::copy(buffer->intensities_.begin(), buffer->intensities_.end(),
              composite_sinusoidal_modeling->begin() + num_sine_wave_);
  }

  return true;
}

}  // namespace sptk
