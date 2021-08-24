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

#ifndef SPTK_MATH_GAUSSIAN_MIXTURE_MODELING_H_
#define SPTK_MATH_GAUSSIAN_MIXTURE_MODELING_H_

#include <vector>  // std::vector

#include "SPTK/math/symmetric_matrix.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Estimate model parameters of GMM.
 *
 * The input is the @f$M@f$-th order input vectors:
 * @f[
 *   \begin{array}{cccc}
 *     \boldsymbol{x}_0, & \boldsymbol{x}_1, & \ldots, & \boldsymbol{x}_{T-1},
 *   \end{array}
 * @f]
 * where @f$T@f$ is the number of vectors.
 * The output is the set of GMM parameters @f$\boldsymbol{\lambda}@f$:
 * @f[
 *   \begin{array}{ccccccc}
 *     w_0, & \boldsymbol{\mu}_0, & \boldsymbol{\varSigma}_0, & \ldots, &
 *     w_{K-1}, & \boldsymbol{\mu}_{K-1}, & \boldsymbol{\varSigma}_{K-1}
 *   \end{array}
 * @f]
 * where @f$K@f$ is the number of mixture components, @f$w_k@f$ is the
 * @f$k@f$-th mixture weight, @f$\mu_k@f$ is the @f$k@f$-th mean vector, and
 * @f$\boldsymbol{\varSigma}_k@f$ is the @f$k@f$-th covariance matrix.
 * The mixture weights satisfy
 * @f[
 *   \sum_{k=0}^{K-1} w_k = 1.
 * @f]
 * The covariance matrix can be full, diagonal, block-diagonal, or block-wise
 * diagonal.
 *
 * The @f$\boldsymbol{\lambda}@f$ is iteratively updated by the following
 * update formulae:
 * @f{eqnarray}{
 *   \hat{w}_k &=&
 *      \frac{1}{T} \sum_{t=0}^{T-1} \gamma_{k,t}, \\
 *   \boldsymbol{\mu}_k &=&
 *      \frac{\displaystyle\sum_{t=0}^{T-1} \gamma_{k,t} \boldsymbol{x}_t}
 *           {\displaystyle\sum_{t=0}^{T-1} \gamma_{k,t}}, \\
 *   \boldsymbol{\varSigma}_k &=&
 *      \frac{\displaystyle\sum_{t=0}^{T-1} \gamma_{k,t}
 *            \boldsymbol{x}_t \boldsymbol{x}_t^{\mathsf{T}}}
 *           {\displaystyle\sum_{t=0}^{T-1} \gamma_{k,t}}
 *      - \boldsymbol{\mu}_t \boldsymbol{\mu}_t^{\mathsf{T}},
 * @f}
 * where
 * @f[
 *   \gamma_{k,t} =
 *     \frac{w_k \mathcal{N}(
 *       \boldsymbol{x}_t \,|\, \boldsymbol{\mu}_k, \boldsymbol{\varSigma}_k)}
 *    {\displaystyle\sum_{k=0}^{K-1} w_k \mathcal{N}(
 *       \boldsymbol{x}_t \,|\, \boldsymbol{\mu}_k, \boldsymbol{\varSigma}_k)}
 * @f]
 * is the posterior probability of being in the @f$k@f$-th component at time
 * @f$t@f$.
 *
 * If the universal background model (UBM) @f$\boldsymbol{\lambda}'@f$ is given,
 * the @f$\boldsymbol{\lambda}@f$ is estimated by the maximum a posteriori
 * method. The joint prior density is the product of Dirichlet and
 * normal-Wishart
 * densities. The update formulae are obtained as
 * @f{eqnarray}{
 *   \hat{w}_k &=&
 *      \frac{\xi_k + \displaystyle\sum_{t=0}^{T-1} \gamma_{k,t}}
 *           {\displaystyle\sum_{k=0}^{K-1} \xi_k + T}, \\
 *   \boldsymbol{\mu}_k &=&
 *      \frac{\xi_k \boldsymbol{\mu}'_k +
 *            \displaystyle\sum_{t=0}^{T-1} \gamma_{k,t} \boldsymbol{x}_t}
 *           {\xi_k + \displaystyle\sum_{t=0}^{T-1} \gamma_{k,t}}, \\
 *   \boldsymbol{\varSigma}_k &=&
 *      \frac{\xi_k \boldsymbol{\varSigma}'_k +
 *            \xi_k (\boldsymbol{\mu}'_k - \boldsymbol{\mu}_k)
 *                  (\boldsymbol{\mu}'_k - \boldsymbol{\mu}_k)^{\mathsf{T}} +
 *            \displaystyle\sum_{t=0}^{T-1} \gamma_{k,t}
 *                  (\boldsymbol{x}_t - \boldsymbol{\mu}_k)
 *                  (\boldsymbol{x}_t - \boldsymbol{\mu}_k)^{\mathsf{T}}}
 *           {\xi_k + \displaystyle\sum_{t=0}^{T-1} \gamma_{k,t}}.
 * @f}
 * where
 * @f[
 *   \xi_k = \alpha w'_k.
 * @f]
 * and @f$\alpha@f$ controlls the importance of the UBM.
 */
class GaussianMixtureModeling {
 public:
  /**
   * Type of (block) covariance.
   */
  enum CovarianceType {
    kDiagonal = 0,
    kFull,
  };

  /**
   * Type of initialization.
   */
  enum InitializationType {
    kNone = 0,
    kKMeans,
    kUbm,
  };

  /**
   * Buffer for GaussianMixtureModeling.
   */
  class Buffer {
   public:
    Buffer() : precomputed_(false) {
    }

    virtual ~Buffer() {
    }

   private:
    std::vector<double> d_;
    std::vector<double> gconsts_;
    std::vector<SymmetricMatrix> precisions_;
    bool precomputed_;

    friend class GaussianMixtureModeling;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_order Order of vector, @f$M@f$.
   * @param[in] num_mixture Number of mixtures, @f$K@f$.
   * @param[in] num_iteration Number of iterations, @f$N@f$.
   * @param[in] convergence_threshold Convergence threshold.
   * @param[in] covariance_type Type of covariance.
   * @param[in] block_size Block size of covariance.
   * @param[in] weight_floor Floor value of weight.
   * @param[in] variance_floor Floor value of variance.
   * @param[in] initialization_type Type of initialization.
   * @param[in] log_interval Show log-likelihood every this step.
   * @param[in] smoothing_parameter MAP hyperparameter (optional), @f$\alpha@f$.
   * @param[in] ubm_weights Weights of UBM-GMM (optional).
   * @param[in] ubm_mean_vectors Means of UBM-GMM (optional).
   * @param[in] ubm_covariance_matrices Covariances of UBM-GMM (optional).
   */
  GaussianMixtureModeling(
      int num_order, int num_mixture, int num_iteration,
      double convergence_threshold, CovarianceType covariance_type,
      std::vector<int> block_size, double weight_floor, double variance_floor,
      InitializationType initialization_type, int log_interval,
      double smoothing_parameter = 0.0,
      const std::vector<double>& ubm_weights = {},
      const std::vector<std::vector<double> >& ubm_mean_vectors = {},
      const std::vector<SymmetricMatrix>& ubm_covariance_matrices = {});

  virtual ~GaussianMixtureModeling() {
  }

  /**
   * @return Order of vector.
   */
  int GetNumOrder() const {
    return num_order_;
  }

  /**
   * @return Number of mixture components.
   */
  int GetNumMixture() const {
    return num_mixture_;
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
   * @return Floor value of weight.
   */
  double GetWeightFloor() const {
    return weight_floor_;
  }

  /**
   * @return Floor value of variance.
   */
  double GetVarianceFloor() const {
    return variance_floor_;
  }

  /**
   * @return Type of initialization.
   */
  InitializationType GetInitializationType() const {
    return initialization_type_;
  }

  /**
   * @return MAP smoothing parameter.
   */
  double GetSmoothingParameter() const {
    return smoothing_parameter_;
  }

  /**
   * @return True if covariance is pure diagonal.
   */
  bool IsDiagonal() const {
    return is_diagonal_;
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
   * @param[in,out] weights @f$K@f$ mixture weights.
   * @param[in,out] mean_vectors @f$K@f$ mean vectors.
   *                The shape is @f$[K, M+1]@f$.
   * @param[in,out] covariance_matrices @f$K@f$ covariance matrices.
   *                The shape is @f$[K, M+1, M+1]@f$.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<std::vector<double> >& input_vectors,
           std::vector<double>* weights,
           std::vector<std::vector<double> >* mean_vectors,
           std::vector<SymmetricMatrix>* covariance_matrices) const;

  /**
   * Calculate log-probablity of data.
   *
   * @param[in] num_order Order of input vector.
   * @param[in] num_mixture Number of mixture components.
   * @param[in] is_diagonal If true, diagonal covariance is assumed.
   * @param[in] check_size If true, check sanity of input GMM parameters.
   * @param[in] input_vector @f$M@f$-th order input vector.
   * @param[in] weights @f$K@f$ mixture weights.
   * @param[in] mean_vectors @f$K@f$ mean vectors.
   * @param[in] covariance_matrices @f$K@f$ covariance matrices.
   * @param[out] components_of_log_probability Components of log-probability.
   * @param[out] log_probability Log-probability of input vector.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  static bool CalculateLogProbability(
      int num_order, int num_mixture, bool is_diagonal, bool check_size,
      const std::vector<double>& input_vector,
      const std::vector<double>& weights,
      const std::vector<std::vector<double> >& mean_vectors,
      const std::vector<SymmetricMatrix>& covariance_matrices,
      std::vector<double>* components_of_log_probability,
      double* log_probability, GaussianMixtureModeling::Buffer* buffer);

 private:
  void FloorWeight(std::vector<double>* weights) const;

  void FloorVariance(std::vector<SymmetricMatrix>* covariance_matrices) const;

  bool Initialize(const std::vector<std::vector<double> >& input_vectors,
                  std::vector<double>* weights,
                  std::vector<std::vector<double> >* mean_vectors,
                  std::vector<SymmetricMatrix>* covariance_matrices) const;

  const int num_order_;
  const int num_mixture_;
  const int num_iteration_;
  const double convergence_threshold_;
  const CovarianceType covariance_type_;
  const std::vector<int> block_size_;
  const double weight_floor_;
  const double variance_floor_;
  const InitializationType initialization_type_;
  const int log_interval_;

  const double smoothing_parameter_;
  const std::vector<double> ubm_weights_;
  const std::vector<std::vector<double> > ubm_mean_vectors_;
  const std::vector<SymmetricMatrix> ubm_covariance_matrices_;

  const bool is_diagonal_;
  bool is_valid_;

  SymmetricMatrix mask_;
  std::vector<double> xi_;

  DISALLOW_COPY_AND_ASSIGN(GaussianMixtureModeling);
};

}  // namespace sptk

#endif  // SPTK_MATH_GAUSSIAN_MIXTURE_MODELING_H_
