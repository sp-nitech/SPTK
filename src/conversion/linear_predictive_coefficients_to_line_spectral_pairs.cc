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

#include "SPTK/conversion/linear_predictive_coefficients_to_line_spectral_pairs.h"

#include <cmath>    // std::acos, std::ceil, std::fabs, std::floor
#include <cstddef>  // std::size_t

namespace {

double CalculateChebyshevPolynomial(const std::vector<double>& coefficients,
                                    double x) {
  const double* c(&coefficients[0]);
  double b2(0.0);
  double b1(0.0);
  for (int i(static_cast<int>(coefficients.size()) - 1); 0 < i; --i) {
    const double b0(2.0 * x * b1 - b2 + c[i]);
    b2 = b1;
    b1 = b0;
  }
  return x * b1 - b2 + c[0];
}

}  // namespace

namespace sptk {

LinearPredictiveCoefficientsToLineSpectralPairs::
    LinearPredictiveCoefficientsToLineSpectralPairs(
        int num_order, int num_split, int num_iteration,
        double convergence_threshold)
    : num_order_(num_order),
      num_symmetric_polynomial_order_(
          static_cast<int>(std::ceil(num_order_ * 0.5))),
      num_asymmetric_polynomial_order_(
          static_cast<int>(std::floor(num_order_ * 0.5))),
      num_split_(num_split),
      num_iteration_(num_iteration),
      convergence_threshold_(convergence_threshold),
      is_valid_(true) {
  if (num_order_ < 0 || num_split_ <= 0 || num_iteration_ <= 0 ||
      convergence_threshold_ < 0.0) {
    is_valid_ = false;
    return;
  }
}

bool LinearPredictiveCoefficientsToLineSpectralPairs::Run(
    const std::vector<double>& linear_predictive_coefficients,
    std::vector<double>* line_spectral_pairs,
    LinearPredictiveCoefficientsToLineSpectralPairs::Buffer* buffer) const {
  // Check inputs.
  if (!is_valid_ ||
      linear_predictive_coefficients.size() !=
          static_cast<std::size_t>(num_order_ + 1) ||
      NULL == line_spectral_pairs || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  if (line_spectral_pairs->size() != static_cast<std::size_t>(num_order_ + 1)) {
    line_spectral_pairs->resize(num_order_ + 1);
  }
  if (buffer->c1_.size() !=
      static_cast<std::size_t>(num_symmetric_polynomial_order_ + 1)) {
    buffer->c1_.resize(num_symmetric_polynomial_order_ + 1);
  }
  if (buffer->c2_.size() !=
      static_cast<std::size_t>(num_asymmetric_polynomial_order_ + 1)) {
    buffer->c2_.resize(num_asymmetric_polynomial_order_ + 1);
  }

  // Copy gain.
  (*line_spectral_pairs)[0] = linear_predictive_coefficients[0];
  if (0 == num_order_) return true;

  // Calculate symmetric and antisymmetric polynomials.
  double* c1(&buffer->c1_[0]);
  double* c2(&buffer->c2_[0]);
  {
    c1[num_symmetric_polynomial_order_] = 1.0;
    c2[num_asymmetric_polynomial_order_] = 1.0;
    const double* p1(&(linear_predictive_coefficients[0]) + 1);
    const double* p2(&(linear_predictive_coefficients[0]) + num_order_);
    if (num_order_ % 2 == 0) {
      for (int i(num_symmetric_polynomial_order_ - 1); 0 <= i;
           --i, ++p1, --p2) {
        c1[i] = *p1 + *p2 - c1[i + 1];
        c2[i] = *p1 - *p2 + c2[i + 1];
      }
    } else {
      for (int i(num_asymmetric_polynomial_order_ - 1); 0 <= i;
           --i, ++p1, --p2) {
        c1[i + 1] = *p1 + *p2;
        c2[i] = (i == num_asymmetric_polynomial_order_ - 1)
                    ? *p1 - *p2
                    : *p1 - *p2 + c2[i + 2];
      }
      c1[0] = *p1 + *p2;
    }
    c1[0] *= 0.5;
    c2[0] *= 0.5;
  }

  std::vector<double>* c(&buffer->c1_);
  int order(0);
  double x_prev(1.0);
  double y_prev(CalculateChebyshevPolynomial(*c, x_prev));
  double* w(&((*line_spectral_pairs)[0]));

  // Search roots of polynomials.
  const double delta(1.0 / num_split_);
  const double x_max(1.0 - delta);
  const double x_min(-1.0 - delta);
  for (double x(x_max); x_min < x; x -= delta) {
    double y(CalculateChebyshevPolynomial(*c, x));

    if (y * y_prev <= 0.0) {
      double x_lower(x);
      double x_upper(x_prev);
      double y_lower(y);
      double y_upper(y_prev);

      for (int n(0); n < num_iteration_; ++n) {
        const double x_mid((x_lower + x_upper) * 0.5);
        const double y_mid(CalculateChebyshevPolynomial(*c, x_mid));

        if (y_mid * y_upper <= 0.0) {
          x_lower = x_mid;
          y_lower = y_mid;
        } else {
          x_upper = x_mid;
          y_upper = y_mid;
        }

        if (std::fabs(y_mid) <= convergence_threshold_) {
          break;
        }
      }

      const double x_interpolated((y_lower * x_upper - y_upper * x_lower) /
                                  (y_lower - y_upper));
      w[++order] = std::acos(x_interpolated) / sptk::kTwoPi;
      if (num_order_ == order) return true;

      // Update variables.
      c = (c == &buffer->c1_) ? &buffer->c2_ : &buffer->c1_;
      x = x_interpolated;
      y = CalculateChebyshevPolynomial(*c, x);
    }

    x_prev = x;
    y_prev = y;
  }

  return false;
}

bool LinearPredictiveCoefficientsToLineSpectralPairs::Run(
    std::vector<double>* input_and_output,
    LinearPredictiveCoefficientsToLineSpectralPairs::Buffer* buffer) const {
  if (NULL == input_and_output) return false;
  return Run(*input_and_output, input_and_output, buffer);
}

}  // namespace sptk
