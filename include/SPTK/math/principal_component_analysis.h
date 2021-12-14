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
   * Type of covariance.
   */
  enum CovarianceType {
    kSampleCovariance = 0,
    kUnbiasedCovariance,
    kCorrelation,
    kNumCovarianceTypes,
  };

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
   * @param[in] covariance_type Type of covariance.
   */
  PrincipalComponentAnalysis(int num_order, int num_iteration,
                             double convergence_threshold,
                             CovarianceType covariance_type);

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
   * @return Type of covariance.
   */
  CovarianceType GetCovarianceType() const {
    return covariance_type_;
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
  const CovarianceType covariance_type_;

  const StatisticsAccumulation accumulation_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(PrincipalComponentAnalysis);
};

}  // namespace sptk

#endif  // SPTK_MATH_PRINCIPAL_COMPONENT_ANALYSIS_H_
