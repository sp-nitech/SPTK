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

#ifndef SPTK_MATH_VANDERMONDE_SYSTEM_SOLVER_H_
#define SPTK_MATH_VANDERMONDE_SYSTEM_SOLVER_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Solve the following Vandermonde system:
 * @f[
 *   \left[ \begin{array}{cccc}
 *     1      & 1      & \cdots & 1      \\
 *     x(0)   & x(1)   & \cdots & x(M)   \\
 *     x^2(0) & x^2(1) & \cdots & x^2(M) \\
 *     \vdots & \vdots & \ddots & \vdots \\
 *     x^M(0) & x^M(1) & \cdots & x^M(M)
 *   \end{array} \right]
 *   \left[ \begin{array}{c}
 *     w(0)   \\
 *     w(1)   \\
 *     w(2)   \\
 *     \vdots \\
 *     w(M)
 *   \end{array} \right] =
 *   \left[ \begin{array}{c}
 *     q(0)   \\
 *     q(1)   \\
 *     q(2)   \\
 *     \vdots \\
 *     q(M)
 *   \end{array} \right].
 * @f]
 *
 * The inputs are
 * @f[
 *   \begin{array}{cccc}
 *     x(0), & x(1), & \ldots, & x(M),
 *   \end{array}
 * @f]
 * and
 * @f[
 *   \begin{array}{cccc}
 *     q(0), & q(1), & \ldots, & q(M).
 *   \end{array}
 * @f]
 * The outputs are the unknown coefficients
 * @f[
 *   \begin{array}{cccc}
 *     w(0), & w(1), & \ldots, & w(M).
 *   \end{array}
 * @f]
 *
 * [1] W. H. Press, et al., &quot;Numerical recipes in C: The art of scientific
 *     computing,&quot; Cambridge University Press, pp. 90-92, 1992.
 */
class VandermondeSystemSolver {
 public:
  /**
   * Buffer for VandermondeSystemSolver class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    std::vector<double> d_;

    friend class VandermondeSystemSolver;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_order Order of vector, @f$M@f$.
   */
  explicit VandermondeSystemSolver(int num_order);

  virtual ~VandermondeSystemSolver() {
  }

  /**
   * @return Order of vector.
   */
  int GetNumOrder() const {
    return num_order_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] coefficient_vector @f$M@f$-th order vector @f$\boldsymbol{x}@f$.
   * @param[in] constant_vector @f$M@f$-th order vector @f$\boldsymbol{q}@f$.
   * @param[out] solution_vector @f$M@f$-th order vector @f$\boldsymbol{w}@f$.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& coefficient_vector,
           const std::vector<double>& constant_vector,
           std::vector<double>* solution_vector,
           VandermondeSystemSolver::Buffer* buffer) const;

 private:
  const int num_order_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(VandermondeSystemSolver);
};

}  // namespace sptk

#endif  // SPTK_MATH_VANDERMONDE_SYSTEM_SOLVER_H_
