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
  for (int m(0); m < num_order_; ++m) {
    const double angle(unit_angle * m + phi);
    cosine_table_[m] = std::cos(angle);
    sine_table_[m] = std::sin(angle);
  }
}

bool DurandKernerMethod::Run(const std::vector<double>& coefficients,
                             std::vector<std::complex<double> >* roots,
                             bool* is_converged) const {
  // Check inputs.
  if (!is_valid_ ||
      coefficients.size() != static_cast<std::size_t>(num_order_) ||
      NULL == roots || NULL == is_converged) {
    return false;
  }

  // Prepare memories.
  if (roots->size() != static_cast<std::size_t>(num_order_)) {
    roots->resize(num_order_);
  }

  const double* a(&(coefficients[0]));
  std::complex<double>* x(&((*roots)[0]));

  *is_converged = false;

  // Set initial roots using the Aberth's approach.
  {
    double radius(0.0);
    for (int m(1); m < num_order_; ++m) {
      const double r(2.0 * std::pow(std::fabs(a[m]), 1.0 / (m + 1)));
      if (radius < r) {
        radius = r;
      }
    }

    const double center(-a[0] / num_order_);
    for (int m(0); m < num_order_; ++m) {
      x[m].real(center + radius * cosine_table_[m]);
      x[m].imag(center + radius * sine_table_[m]);
    }
  }

  // Find roots using the Durand-Kerner method.
  for (int n(0); n < num_iteration_; ++n) {
    bool halt(true);

    for (int m(0); m < num_order_; ++m) {
      std::complex<double> numerator(1.0);
      std::complex<double> denominator(1.0);
      for (int l(0); l < num_order_; ++l) {
        numerator = numerator * x[m] + std::complex<double>(a[l]);
        if (m != l) denominator = denominator * (x[m] - x[l]);
      }
      if (0.0 == denominator.real() && 0.0 == denominator.imag()) {
        x[m] = 0.0;
      } else {
        const std::complex<double> delta(numerator / denominator);
        x[m] -= delta;
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
