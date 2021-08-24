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

#include "SPTK/conversion/line_spectral_pairs_to_linear_predictive_coefficients.h"

#include <algorithm>  // std::fill_n
#include <cmath>      // std::ceil, std::cos, std::floor
#include <cstddef>    // std::size_t

namespace sptk {

LineSpectralPairsToLinearPredictiveCoefficients::
    LineSpectralPairsToLinearPredictiveCoefficients(int num_order)
    : num_order_(num_order),
      num_symmetric_polynomial_order_(
          static_cast<int>(std::ceil(num_order_ * 0.5))),
      num_asymmetric_polynomial_order_(
          static_cast<int>(std::floor(num_order_ * 0.5))),
      is_valid_(true) {
  if (num_order_ < 0) {
    is_valid_ = false;
    return;
  }
}

bool LineSpectralPairsToLinearPredictiveCoefficients::Run(
    const std::vector<double>& line_spectral_pairs,
    std::vector<double>* linear_predictive_coefficients,
    LineSpectralPairsToLinearPredictiveCoefficients::Buffer* buffer) const {
  // Check inputs.
  if (!is_valid_ ||
      line_spectral_pairs.size() != static_cast<std::size_t>(num_order_ + 1) ||
      NULL == linear_predictive_coefficients || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  if (linear_predictive_coefficients->size() !=
      static_cast<std::size_t>(num_order_ + 1)) {
    linear_predictive_coefficients->resize(num_order_ + 1);
  }
  if (buffer->p_.size() !=
      static_cast<std::size_t>(num_asymmetric_polynomial_order_)) {
    buffer->p_.resize(num_asymmetric_polynomial_order_);
  }
  if (buffer->q_.size() !=
      static_cast<std::size_t>(num_symmetric_polynomial_order_)) {
    buffer->q_.resize(num_symmetric_polynomial_order_);
  }
  if (buffer->a0_.size() !=
      static_cast<std::size_t>(num_asymmetric_polynomial_order_ + 1)) {
    buffer->a0_.resize(num_asymmetric_polynomial_order_ + 1);
  }
  if (buffer->a1_.size() !=
      static_cast<std::size_t>(num_asymmetric_polynomial_order_)) {
    buffer->a1_.resize(num_asymmetric_polynomial_order_);
  }
  if (buffer->a2_.size() !=
      static_cast<std::size_t>(num_asymmetric_polynomial_order_)) {
    buffer->a2_.resize(num_asymmetric_polynomial_order_);
  }
  if (buffer->b0_.size() !=
      static_cast<std::size_t>(num_symmetric_polynomial_order_ + 1)) {
    buffer->b0_.resize(num_symmetric_polynomial_order_ + 1);
  }
  if (buffer->b1_.size() !=
      static_cast<std::size_t>(num_symmetric_polynomial_order_)) {
    buffer->b1_.resize(num_symmetric_polynomial_order_);
  }
  if (buffer->b2_.size() !=
      static_cast<std::size_t>(num_symmetric_polynomial_order_)) {
    buffer->b2_.resize(num_symmetric_polynomial_order_);
  }

  // Copy gain.
  (*linear_predictive_coefficients)[0] = line_spectral_pairs[0];
  if (0 == num_order_) return true;

  const double* w(&(line_spectral_pairs[0]));
  double* a(&((*linear_predictive_coefficients)[0]));
  double* p(&buffer->p_[0]);
  double* q(&buffer->q_[0]);
  double* a0(&buffer->a0_[0]);
  double* a1(&buffer->a1_[0]);
  double* a2(&buffer->a2_[0]);
  double* b0(&buffer->b0_[0]);
  double* b1(&buffer->b1_[0]);
  double* b2(&buffer->b2_[0]);
  double c0(1.0);
  double c1(0.0);
  double c2(0.0);

  // Calculate line spectral pairs digital filter parameters.
  for (int i(0), j(2); i < num_asymmetric_polynomial_order_; ++i, j += 2) {
    p[i] = -2.0 * std::cos(w[j]);
  }
  for (int i(0), j(1); i < num_symmetric_polynomial_order_; ++i, j += 2) {
    q[i] = -2.0 * std::cos(w[j]);
  }

  std::fill_n(a1, num_asymmetric_polynomial_order_, 0.0);
  std::fill_n(a2, num_asymmetric_polynomial_order_, 0.0);
  std::fill_n(b1, num_symmetric_polynomial_order_, 0.0);
  std::fill_n(b2, num_symmetric_polynomial_order_, 0.0);

  // Calculate impulse response of analysis filter.
  const bool is_even(sptk::IsEven(num_order_));
  for (int m(0); m <= num_order_; ++m) {
    if (is_even) {
      a0[0] = c0 - c1;
      b0[0] = c0 + c1;
      c1 = c0;
    } else {
      a0[0] = c0 - c2;
      b0[0] = c0;
      c2 = c1;
      c1 = c0;
    }

    for (int i(0); i < num_asymmetric_polynomial_order_; ++i) {
      a0[i + 1] = a0[i] + p[i] * a1[i] + a2[i];
      a2[i] = a1[i];
      a1[i] = a0[i];
    }
    for (int i(0); i < num_symmetric_polynomial_order_; ++i) {
      b0[i + 1] = b0[i] + q[i] * b1[i] + b2[i];
      b2[i] = b1[i];
      b1[i] = b0[i];
    }

    if (0 == m) {
      c0 = 0.0;
    } else {
      a[m] = 0.5 * (a0[num_asymmetric_polynomial_order_] +
                    b0[num_symmetric_polynomial_order_]);
    }
  }

  return true;
}

bool LineSpectralPairsToLinearPredictiveCoefficients::Run(
    std::vector<double>* input_and_output,
    LineSpectralPairsToLinearPredictiveCoefficients::Buffer* buffer) const {
  if (NULL == input_and_output) return false;
  return Run(*input_and_output, input_and_output, buffer);
}

}  // namespace sptk
