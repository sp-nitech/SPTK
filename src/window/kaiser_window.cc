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

#include "SPTK/window/kaiser_window.h"

#include <cmath>  // std::pow, std::sqrt

namespace {

/**
 * Calculate the zeroth-order modified Bessel function of the first kind.
 */
double I0(double x, int upper_limit = 500, double eps = 1e-6) {
  const double y(0.5 * x);
  double z(1.0);
  double sum(1.0);
  double prev_sum(sum);
  for (int i(1); i < upper_limit; ++i) {
    z *= y / i;
    sum += z * z;
    if (sum - prev_sum < eps) break;
    prev_sum = sum;
  }
  return sum;
}

}  // namespace

namespace sptk {

KaiserWindow::KaiserWindow(int window_length, double beta, bool periodic)
    : window_length_(window_length),
      beta_(beta),
      periodic_(periodic),
      is_valid_(true) {
  if (window_length_ <= 0 || beta_ < 0.0) {
    is_valid_ = false;
    return;
  }

  window_.resize(window_length);
  if (1 == window_length_) {
    window_[0] = 1.0;
    return;
  }

  const double a(0.5 * (periodic_ ? window_length_ : window_length_ - 1));
  const double z(1.0 / I0(beta_));
  double* window(&(window_[0]));
  for (int i(0); i < window_length_; ++i) {
    const double x((i - a) / a);
    window[i] = z * I0(beta_ * std::sqrt(1.0 - x * x));
  }
}

double KaiserWindow::AttenuationToBeta(double attenuation) {
  double beta;
  if (attenuation <= 21.0) {
    beta = 0.0;
  } else if (attenuation <= 50.0) {
    const double a(attenuation - 21.0);
    beta = 0.5842 * std::pow(a, 0.4) + 0.07886 * a;
  } else {
    const double a(attenuation - 8.7);
    beta = 0.1102 * a;
  }
  return beta;
}

}  // namespace sptk
