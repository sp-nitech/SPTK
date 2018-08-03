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

#include "SPTK/converter/linear_predictive_coefficients_to_line_spectral_pairs.h"

#include <cmath>    // std::acos, std::ceil, std::fabs, std::floor
#include <cstddef>  // std::size_t

namespace {

bool CalculateChebyshevPolynomial(const std::vector<double>& coefficients,
                                  double x, double* y) {
  if (coefficients.empty() || NULL == y) {
    return false;
  }

  const double* c(&coefficients[0]);
  double b2(0.0);
  double b1(0.0);
  for (int i(coefficients.size() - 1); 0 < i; --i) {
    const double b0(2.0 * x * b1 - b2 + c[i]);
    b2 = b1;
    b1 = b0;
  }
  *y = x * b1 - b2 + c[0];

  return true;
}

}  // namespace

namespace sptk {

LinearPredictiveCoefficientsToLineSpectralPairs::
    LinearPredictiveCoefficientsToLineSpectralPairs(int num_order,
                                                    int num_split,
                                                    int num_iteration,
                                                    double epsilon)
    : num_order_(num_order),
      num_symmetric_polynomial_order_(
          static_cast<int>(std::ceil(num_order_ * 0.5))),
      num_asymmetric_polynomial_order_(
          static_cast<int>(std::floor(num_order_ * 0.5))),
      num_split_(num_split),
      num_iteration_(num_iteration),
      epsilon_(epsilon),
      is_valid_(true) {
  if (num_order_ < 0 || num_split_ <= 0 || num_iteration_ <= 0 ||
      epsilon_ < 0.0) {
    is_valid_ = false;
  }
}

bool LinearPredictiveCoefficientsToLineSpectralPairs::Run(
    const std::vector<double>& linear_predictive_coefficients,
    std::vector<double>* line_spectral_pairs,
    LinearPredictiveCoefficientsToLineSpectralPairs::Buffer* buffer) const {
  // check inputs
  if (!is_valid_ ||
      linear_predictive_coefficients.size() !=
          static_cast<std::size_t>(num_order_ + 1) ||
      NULL == line_spectral_pairs || NULL == buffer) {
    return false;
  }

  // prepare memory
  if (line_spectral_pairs->size() != static_cast<std::size_t>(num_order_ + 1)) {
    line_spectral_pairs->resize(num_order_ + 1);
  }

  (*line_spectral_pairs)[0] = linear_predictive_coefficients[0];
  if (0 == num_order_) return true;

  // prepare buffer
  if (buffer->c1_.size() !=
      static_cast<std::size_t>(num_symmetric_polynomial_order_ + 1)) {
    buffer->c1_.resize(num_symmetric_polynomial_order_ + 1);
  }
  if (buffer->c2_.size() !=
      static_cast<std::size_t>(num_asymmetric_polynomial_order_ + 1)) {
    buffer->c2_.resize(num_asymmetric_polynomial_order_ + 1);
  }

  // calculate symmetric and antisymmetric polynomials
  const double* p1(&(linear_predictive_coefficients[0]) + 1);
  const double* p2(&(linear_predictive_coefficients[0]) + num_order_);
  double* c1(&buffer->c1_[0]);
  double* c2(&buffer->c2_[0]);
  c1[num_symmetric_polynomial_order_] = 1.0;
  c2[num_asymmetric_polynomial_order_] = 1.0;
  if (num_order_ % 2 == 0) {
    for (int i(num_symmetric_polynomial_order_ - 1); 0 <= i; --i, ++p1, --p2) {
      c1[i] = *p1 + *p2 - c1[i + 1];
      c2[i] = *p1 - *p2 + c2[i + 1];
    }
  } else {
    for (int i(num_asymmetric_polynomial_order_ - 1); 0 <= i; --i, ++p1, --p2) {
      c1[i + 1] = *p1 + *p2;
      c2[i] = (i == num_asymmetric_polynomial_order_ - 1)
                  ? *p1 - *p2
                  : *p1 - *p2 + c2[i + 2];
    }
    c1[0] = *p1 + *p2;
  }
  c1[0] *= 0.5;
  c2[0] *= 0.5;

  // set initial condition
  int order(0);
  std::vector<double>* c(&buffer->c1_);
  double x_prev(1.0);
  double y_prev;
  if (!CalculateChebyshevPolynomial(*c, x_prev, &y_prev)) return false;

  // search roots of polynomials
  const double delta(1.0 / num_split_);
  const double x_max(1.0 - delta);
  const double x_min(-1.0 - delta);
  for (double x(x_max); x_min < x; x -= delta) {
    double y;
    if (!CalculateChebyshevPolynomial(*c, x, &y)) return false;

    if (y * y_prev <= 0.0) {
      double x_lower(x);
      double x_upper(x_prev);
      double y_lower(y);
      double y_upper(y_prev);

      for (int i(0); i < num_iteration_; ++i) {
        double x_mid((x_lower + x_upper) * 0.5);
        double y_mid;
        if (!CalculateChebyshevPolynomial(*c, x_mid, &y_mid)) return false;

        if (y_mid * y_upper <= 0.0) {
          x_lower = x_mid;
          y_lower = y_mid;
        } else {
          x_upper = x_mid;
          y_upper = y_mid;
        }

        if (std::fabs(y_mid) <= epsilon_) break;
      }

      const double x_interpolated((y_lower * x_upper - y_upper * x_lower) /
                                  (y_lower - y_upper));
      (*line_spectral_pairs)[++order] =
          std::acos(x_interpolated) / sptk::kTwoPi;
      if (num_order_ == order) return true;

      // update variables
      c = (c == &buffer->c1_) ? &buffer->c2_ : &buffer->c1_;
      x = x_interpolated;
      if (!CalculateChebyshevPolynomial(*c, x, &y)) return false;
    }

    x_prev = x;
    y_prev = y;
  }

  return false;
}

}  // namespace sptk
