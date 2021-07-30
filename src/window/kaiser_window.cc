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
//                1996-2021  Nagoya Institute of Technology          //
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
