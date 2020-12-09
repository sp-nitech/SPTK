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

#ifndef SPTK_MATH_PRINCIPAL_COMPONENT_ANALYSIS_H_
#define SPTK_MATH_PRINCIPAL_COMPONENT_ANALYSIS_H_

#include <vector>  // std::vector

#include "SPTK/math/matrix.h"
#include "SPTK/math/statistics_accumulation.h"
#include "SPTK/math/symmetric_matrix.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Perform principal component analysis.
 *
 * The input is the @f$M@f$-th order vectors:
 * @f[
 *   \begin{array}{cccc}
 *     \boldsymbol{x}(0), & \boldsymbol{x}(1), & \ldots, & \boldsymbol{x}(T-1),
 *   \end{array}
 * @f]
 * and the outputs are the @f$M@f$-th order mean vector
 * @f[
 *   \boldsymbol{m} = \frac{1}{T} \sum_{t=0}^{T-1} \boldsymbol{x}(t),
 * @f]
 * the @f$M@f$-th order eigenvectors
 * @f[
 *   \begin{array}{cccc}
 *     \boldsymbol{v}(0), & \boldsymbol{v}(1), & \ldots, & \boldsymbol{v}(M),
 *   \end{array}
 * @f]
 * and the corresponding eigenvalues:
 * @f[
 *   \begin{array}{cccc}
 *     \lambda(0), & \lambda(1), & \ldots, & \lambda(M).
 *   \end{array}
 * @f]
 * The eigenvalue problem is solved by the Jacobi iterative method.
 */
class PrincipalComponentAnalysis {
 public:
  /**
   * Buffer for PrincipalComponentAnalysis class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    StatisticsAccumulation::Buffer buffer_for_accumulation;
    SymmetricMatrix a_;
    std::vector<int> order_of_eigenvalue_;

    friend class PrincipalComponentAnalysis;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_order Order of vector, @f$M@f$.
   * @param[in] num_iteration Number of iterations.
   * @param[in] convergence_threshold Convergence threshold.
   */
  PrincipalComponentAnalysis(int num_order, int num_iteration,
                             double convergence_threshold);

  virtual ~PrincipalComponentAnalysis() {
  }

  /**
   * @return Order of vector.
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
   * @param[in] input_vectors @f$M@f$-th order input vectors.
   *            The shape is @f$[T, M+1]@f$.
   * @param[out] mean_vector @f$M@f$-th order mean vector.
   * @param[out] eigenvalues @f$M+1@f$ eigenvalues.
   * @param[out] eigenvectors @f$M@f$-th order eigenvectors.
   *             The shape is @f$[M+1, M+1]@f$.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<std::vector<double> >& input_vectors,
           std::vector<double>* mean_vector, std::vector<double>* eigenvalues,
           Matrix* eigenvectors,
           PrincipalComponentAnalysis::Buffer* buffer) const;

 private:
  const int num_order_;
  const int num_iteration_;
  const double convergence_threshold_;

  const StatisticsAccumulation accumulation_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(PrincipalComponentAnalysis);
};

}  // namespace sptk

#endif  // SPTK_MATH_PRINCIPAL_COMPONENT_ANALYSIS_H_
