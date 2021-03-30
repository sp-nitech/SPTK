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

#ifndef SPTK_MATH_TOEPLITZ_PLUS_HANKEL_SYSTEM_SOLVER_H_
#define SPTK_MATH_TOEPLITZ_PLUS_HANKEL_SYSTEM_SOLVER_H_

#include <vector>  // std::vector

#include "SPTK/math/matrix2d.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Solve the following Toeplitz-plus-Hankel system:
 * @f[
 *   (\boldsymbol{T} + \boldsymbol{H}) \boldsymbol{x} = \boldsymbol{b},
 * @f]
 * where
 * @f{eqnarray}{
 *   \boldsymbol{T} &=& \left[ \begin{array}{ccccc}
 *     t(0)   & t(-1)   & t(-2)  & \cdots & t(-M)   \\
 *     t(1)   & t(0)    & t(-1)  & \cdots & t(-M+1) \\
 *     t(2)   & t(1)    & t(0)   & \ddots & \vdots  \\
 *     \vdots & \vdots  & \ddots & \ddots & t(-1)   \\
 *     t(M)   & t(M-1)  & \cdots & t(1)   & t(0)
 *   \end{array} \right], \\
 *   \boldsymbol{H} &=& \left[ \begin{array}{ccccc}
 *     h(0)   & h(1)    & h(2)   & \cdots & h(M)    \\
 *     h(1)   & h(2)    & h(3)   & \cdots & h(M+1)  \\
 *     h(2)   & t(3)    & h(4)   & \ddots & \vdots  \\
 *     \vdots & \vdots  & \ddots & \ddots & h(2M-1) \\
 *     h(M)   & h(M+1)  & \cdots & h(2M-1)& h(2M)
 *   \end{array} \right], \\
 *   \boldsymbol{b} &=& \left[ \begin{array}{cccc}
 *     b(0)   & b(1)    & \cdots & b(M)
 *   \end{array} \right]^\mathsf{T}. \\
 * @f}
 *
 * The inputs are @f$(2M+1)@f$ Toeplitz coefficient vector:
 * @f[
 *   \begin{array}{cccc}
 *     t(-M), & t(-M+1), & \ldots, & t(M),
 *   \end{array}
 * @f]
 * @f$(2M+1)@f$ Hankel coefficient vector:
 * @f[
 *   \begin{array}{cccc}
 *     h(0), & h(1), & \ldots, & h(2M),
 *   \end{array}
 * @f]
 * and @f$(M+1)@f$ constant vector:
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
 *
 * [1] G. Merchant and T. Parks, &quot;Efficient solution of a
 *     Toeplitz-plus-Hankel coefficient matrix system of equations,&quot; IEEE
 *     Transactions on Acoustics, Speech, and Signal Processing, vol. 30, no. 1,
 *     pp. 40-44, 1982.
 */
class ToeplitzPlusHankelSystemSolver {
 public:
  /**
   * Buffer for ToeplitzPlusHankelSystemSolver class.
   */
  class Buffer {
   public:
    Buffer() : ep_(2), g_(2), bar_(2), tmp_vector_(2) {
    }

    virtual ~Buffer() {
    }

   private:
    std::vector<Matrix2D> r_;
    std::vector<Matrix2D> x_;
    std::vector<Matrix2D> prev_x_;
    std::vector<std::vector<double>> p_;
    std::vector<double> ep_;
    std::vector<double> g_;
    std::vector<double> bar_;
    std::vector<double> tmp_vector_;
    Matrix2D vx_;
    Matrix2D ex_;
    Matrix2D bx_;
    Matrix2D inv_;
    Matrix2D tau_;
    Matrix2D tmp_matrix_;

    friend class ToeplitzPlusHankelSystemSolver;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_order Order of vector, @f$M@f$.
   * @param[in] coefficients_modification If true, perform coefficients
   *            modification.
   */
  ToeplitzPlusHankelSystemSolver(int num_order,
                                 bool coefficients_modification = true);

  virtual ~ToeplitzPlusHankelSystemSolver() {
  }

  /**
   * @return Order of vector.
   */
  int GetNumOrder() const {
    return num_order_;
  }

  /**
   * @return Ture if coefficients modification is enabled.
   */
  bool GetCoefficientsModificationFlag() {
    return coefficients_modification_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] toeplitz_coefficient_vector @f$2M@f$-th order coefficient vector
   *            of Toeplitz matrix @f$\boldsymbol{T}@f$.
   * @param[in] hankel_coefficient_vector @f$2M@f$-th order coefficient vector
   *            of Hankel matrix @f$\boldsymbol{H}@f$.
   * @param[in] constant_vector @f$M@f$-th order constant vector
   *            @f$\boldsymbol{b}@f$.
   * @param[out] solution_vector @f$M@f$-th order solution vector
   *            @f$\boldsymbol{x}@f$.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& toeplitz_coefficient_vector,
           const std::vector<double>& hankel_coefficient_vector,
           const std::vector<double>& constant_vector,
           std::vector<double>* solution_vector,
           ToeplitzPlusHankelSystemSolver::Buffer* buffer) const;

 private:
  const int num_order_;
  const bool coefficients_modification_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(ToeplitzPlusHankelSystemSolver);
};

}  // namespace sptk

#endif  // SPTK_MATH_TOEPLITZ_PLUS_HANKEL_SYSTEM_SOLVER_H_
