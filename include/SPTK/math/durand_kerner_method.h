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

#ifndef SPTK_MATH_DURAND_KERNER_METHOD_H_
#define SPTK_MATH_DURAND_KERNER_METHOD_H_

#include <complex>  // std::complex
#include <vector>   // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Find roots of a polynomial.
 *
 * The input is the @f$M@f$-th order polynomial coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     a(1), & a(2), & \ldots, & a(M),
 *   \end{array}
 * @f]
 * where the polynomial is represented as
 * @f[
 *   x^M + a(1) x^{M-1} + \cdots + a(M-1) x + a(M).
 * @f]
 * The output is the complex-valued roots of the polynomial:
 * @f[
 *   \begin{array}{cccc}
 *     z(0), & z(1), & \ldots, & z(M-1).
 *   \end{array}
 * @f]
 * They are found by the Durand-Kerner method.
 */
class DurandKernerMethod {
 public:
  /**
   * @param[in] num_order Order of coefficients, @f$M@f$.
   * @param[in] num_iteration Number of iterations.
   * @param[in] convergence_threshold Convergence threshold.
   */
  DurandKernerMethod(int num_order, int num_iteration,
                     double convergence_threshold);

  virtual ~DurandKernerMethod() {
  }

  /**
   * @return Order of coefficients.
   */
  int GetNumOrder() const {
    return num_order_;
  }

  /**
   * @return Number of iterations.
   */
  int GetNumIteration() const {
    return num_iteration_;
  }

  /**
   * @return Convergence threshold.
   */
  double GetConvergenceThreshold() const {
    return convergence_threshold_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] coefficients Coefficients of polynomial.
   * @param[out] roots Root of the polynomial.
   * @param[out] is_converged True if convergence is reached.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& coefficients,
           std::vector<std::complex<double> >* roots, bool* is_converged) const;

 private:
  const int num_order_;
  const int num_iteration_;
  const double convergence_threshold_;

  bool is_valid_;

  std::vector<double> cosine_table_;
  std::vector<double> sine_table_;

  DISALLOW_COPY_AND_ASSIGN(DurandKernerMethod);
};

}  // namespace sptk

#endif  // SPTK_MATH_DURAND_KERNER_METHOD_H_
