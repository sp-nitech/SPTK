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

#ifndef SPTK_MATH_SYMMETRIC_SYSTEM_SOLVER_H_
#define SPTK_MATH_SYMMETRIC_SYSTEM_SOLVER_H_

#include <vector>  // std::vector

#include "SPTK/math/symmetric_matrix.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Solve the following symmetric system:
 * @f[
 *   \boldsymbol{A} \boldsymbol{x} = \boldsymbol{b},
 * @f]
 * where @f$\boldsymbol{A}@f$ is a symmetric matrix.
 *
 * The inputs are @f$\boldsymbol{A}@f$ and @f$M@f$-th order constant vector:
 * @f[
 *   \begin{array}{cccc}
 *     b(0), & b(1), & \ldots, & b(M).
 *   \end{array}
 * @f]
 * The outputs are the unknown coefficients
 * @f[
 *   \begin{array}{cccc}
 *     x(0), & x(1), & \ldots, & x(M).
 *   \end{array}
 * @f]
 */
class SymmetricSystemSolver {
 public:
  /**
   * Buffer for SymmetricSystemSolver class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    SymmetricMatrix inverse_matrix_;

    friend class SymmetricSystemSolver;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_order Order of vector, @f$M@f$.
   */
  explicit SymmetricSystemSolver(int num_order);

  virtual ~SymmetricSystemSolver() {
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
   * @param[in] coefficient_matrix @f$(M+1, M+1)@f$ matrix @f$\boldsymbol{A}@f$.
   * @param[in] constant_vector @f$M@f$-th order vector @f$\boldsymbol{b}@f$.
   * @param[out] solution_vector @f$M@f$-th order vector @f$\boldsymbol{x}@f$.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const SymmetricMatrix& coefficient_matrix,
           const std::vector<double>& constant_vector,
           std::vector<double>* solution_vector,
           SymmetricSystemSolver::Buffer* buffer) const;

 private:
  const int num_order_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(SymmetricSystemSolver);
};

}  // namespace sptk

#endif  // SPTK_MATH_SYMMETRIC_SYSTEM_SOLVER_H_
