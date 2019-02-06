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
//                1996-2019  Nagoya Institute of Technology          //
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

#include "SPTK/math/durand_kerner_method.h"

#include <cmath>    // std::cos, std::fabs, std::pow, std::sin
#include <complex>  // std::abs, std::complex
#include <cstddef>  // std::size_t

namespace sptk {

DurandKernerMethod::DurandKernerMethod(int num_order, int num_iteration,
                                       double convergence_threshold)
    : num_order_(num_order),
      num_iteration_(num_iteration),
      convergence_threshold_(convergence_threshold),
      is_valid_(true) {
  if (num_order_ <= 0 || num_iteration_ <= 0 || convergence_threshold_ < 0.0) {
    is_valid_ = false;
    return;
  }

  cosine_table_.resize(num_order_);
  sine_table_.resize(num_order_);
  const double phi(sptk::kPi / (2 * num_order_));
  const double unit_angle(sptk::kTwoPi / num_order_);
  for (int i(0); i < num_order_; ++i) {
    const double angle(unit_angle * i + phi);
    cosine_table_[i] = std::cos(angle);
    sine_table_[i] = std::sin(angle);
  }
}

bool DurandKernerMethod::Run(const std::vector<double>& coefficients,
                             std::vector<std::complex<double> >* roots,
                             bool* is_converged) const {
  // check inputs
  if (!is_valid_ ||
      coefficients.size() != static_cast<std::size_t>(num_order_) ||
      NULL == roots || NULL == is_converged) {
    return false;
  }

  // prepare memories
  if (roots->size() != static_cast<std::size_t>(num_order_)) {
    roots->resize(num_order_);
  }

  // get values
  const double* a(&(coefficients[0]));
  std::complex<double>* x(&((*roots)[0]));

  // set value
  *is_converged = false;

  // set initial roots using Aberth's approach
  {
    double radius(0.0);
    for (int i(1); i < num_order_; ++i) {
      const double r(2.0 * std::pow(std::fabs(a[i]), 1.0 / (i + 1)));
      if (radius < r) {
        radius = r;
      }
    }

    const double center(-a[0] / num_order_);
    for (int i(0); i < num_order_; ++i) {
      x[i].real(center + radius * cosine_table_[i]);
      x[i].imag(center + radius * sine_table_[i]);
    }
  }

  // find roots using Durand-Kerner method
  for (int i(0); i < num_iteration_; ++i) {
    bool halt(true);
    for (int j(0); j < num_order_; ++j) {
      std::complex<double> numerator(1.0);
      std::complex<double> denominator(1.0);
      for (int k(0); k < num_order_; ++k) {
        numerator = numerator * x[j] + std::complex<double>(a[k]);
        if (j != k) denominator = denominator * (x[j] - x[k]);
      }
      if (0.0 == denominator.real() && 0.0 == denominator.imag()) {
        x[j] = 0.0;
      } else {
        const std::complex<double> delta(numerator / denominator);
        x[j] -= delta;
        if (halt && convergence_threshold_ < std::abs(delta)) {
          halt = false;
        }
      }
    }

    if (halt) {
      *is_converged = true;
      break;
    }
  }

  return true;
}

}  // namespace sptk
