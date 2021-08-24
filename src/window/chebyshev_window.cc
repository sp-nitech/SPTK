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

#include "SPTK/window/chebyshev_window.h"

#include <cmath>  // std::abs, std::acos, std::acosh, std::cos, std::cosh, std::pow

namespace {

/**
 * Calculate chebyshev polynomial.
 */
double T(double x, int n) {
  double t;
  if (std::abs(x) <= 1.0) {
    t = std::cos(n * std::acos(x));
  } else if (1.0 < x) {
    t = std::cosh(n * std::acosh(x));
  } else {
    t = (sptk::IsEven(n) ? 1 : -1) * std::cosh(n * std::acosh(-x));
  }
  return t;
}

}  // namespace

namespace sptk {

ChebyshevWindow::ChebyshevWindow(int window_length, double ripple_ratio,
                                 bool periodic)
    : window_length_(window_length),
      ripple_ratio_(ripple_ratio),
      periodic_(periodic),
      is_valid_(true) {
  if (window_length_ <= 0 || ripple_ratio_ <= 0.0) {
    is_valid_ = false;
    return;
  }

  window_.resize(window_length_);
  if (1 == window_length_) {
    window_[0] = 1.0;
    return;
  }

  const int n(periodic_ ? window_length_ + 1 : window_length_);
  const int n1(n - 1);
  const double s(1.0 / ripple_ratio_);
  const double x0(std::cosh(std::acosh(s) / n1));
  double* window(&(window_[0]));

  if (sptk::IsEven(n)) {
    for (int i(0); i < window_length_; ++i) {
      const int j(2 * i + 1);
      double sum(0.0);
      int sign(1);
      for (int k(0); k <= n1; ++k) {
        const double t(sptk::kPi * k / n);
        sum += sign * T(x0 * std::cos(t), n1) * std::cos(t * j);
        sign *= -1;
      }
      window[i] = sum;
    }
  } else {
    for (int i(0); i < window_length_; ++i) {
      const int m(n1 / 2);
      const int j(2 * (i - m));
      double sum(0.0);
      for (int k(1); k <= m; ++k) {
        const double t(sptk::kPi * k / n);
        sum += T(x0 * std::cos(t), n1) * std::cos(t * j);
      }
      window[i] = s + 2.0 * sum;
    }
  }

  // Normalize to have one as the largest value.
  double z(0.0);
  for (int i(0); i <= window_length_ / 2; ++i) {
    if (z < window[i]) z = window[i];
  }
  z = 1.0 / z;
  for (int i(0); i < window_length_; ++i) {
    window[i] *= z;
  }
}

double ChebyshevWindow::AttenuationToRippleRatio(double attenuation) {
  return 1.0 / std::pow(10.0, attenuation / 20.0);
}

}  // namespace sptk
