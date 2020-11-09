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

#include "SPTK/window/chebyshev_window.h"

#include <cmath>  // std::acos, std::acosh, std::cos, std::cosh, std::pow

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
  if (0 == window_length_ || ripple_ratio_ <= 0.0) {
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
