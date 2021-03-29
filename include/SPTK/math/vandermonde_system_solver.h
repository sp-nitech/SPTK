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
