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

#ifndef SPTK_MATH_GAUSSIAN_MIXTURE_MODEL_BASED_CONVERSION_H_
#define SPTK_MATH_GAUSSIAN_MIXTURE_MODEL_BASED_CONVERSION_H_

#include <vector>  // std::vector

#include "SPTK/generation/nonrecursive_maximum_likelihood_parameter_generation.h"
#include "SPTK/math/matrix.h"
#include "SPTK/math/symmetric_matrix.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Perform GMM-based voice conversion.
 *
 * The input is the @f$(D+1)(M_1+1)@f$-length source vectors:
 * @f[
 *   \begin{array}{cccc}
 *     \boldsymbol{X}_0, & \boldsymbol{X}_1, & \ldots, & \boldsymbol{X}_{T-1},
 *   \end{array}
 * @f]
 * where
 * @f[
 *   \boldsymbol{X}_t = \left[ \begin{array}{cccc}
 *     \boldsymbol{x}_t^{\mathsf{T}} &
 *     \Delta^{(1)} \boldsymbol{x}_t^{\mathsf{T}} & \cdots &
 *     \Delta^{(D)} \boldsymbol{x}_t^{\mathsf{T}}
 *   \end{array} \right]^{\mathsf{T}}.
 * @f]
 * The output is the @f$(M_2+1)@f$-length target vectors:
 * @f[
 *   \begin{array}{cccc}
 *     \boldsymbol{y}_0, & \boldsymbol{y}_1, & \ldots, & \boldsymbol{y}_{T-1}.
 *   \end{array}
 * @f]
 *
 * The optimal target vectors can be drived in a maximum likelihood sense:
 * @f{eqnarray}{
 *   \hat{\boldsymbol{y}} &=& \mathop{\mathrm{argmax}}_{\boldsymbol{y}} \,
 *     p(\boldsymbol{Y} \,|\, \boldsymbol{X}, \boldsymbol{\lambda}) \\
 *   &=& \mathop{\mathrm{argmax}}_{\boldsymbol{y}}
 *     \sum_{\boldsymbol{m}}
 *     p(\boldsymbol{m} \,|\, \boldsymbol{X}, \boldsymbol{\lambda}) \,
 *     p(\boldsymbol{Y} \,|\, \boldsymbol{X}, \boldsymbol{m},
 *                            \boldsymbol{\lambda}),
 * @f}
 * where @f$\boldsymbol{\lambda}@f$ is the GMM that models the joint vectors.
 * The mean vector and the covariance matrix of the @f$m@f$-th mixture component
 * are written as
 * @f[
 *   \boldsymbol{\mu}_m = \left[ \begin{array}{c}
 *     \boldsymbol{\mu}_m^{(X)} \\
 *     \boldsymbol{\mu}_m^{(Y)}
 *   \end{array} \right]
 * @f]
 * and
 * @f[
 *   \boldsymbol{\varSigma}_m = \left[ \begin{array}{cc}
 *     \boldsymbol{\varSigma}_m^{(XX)} & \boldsymbol{\varSigma}_m^{(XY)} \\
 *     \boldsymbol{\varSigma}_m^{(YX)} & \boldsymbol{\varSigma}_m^{(YY)}
 *   \end{array} \right].
 * @f]
 * To easily compute the ML estimate, the maximum a posteriori approximation is
 * applied:
 * @f{eqnarray}{
 *   \hat{\boldsymbol{y}} &=& \mathop{\mathrm{argmax}}_{\boldsymbol{y}} \,
 *     p(\hat{\boldsymbol{m}} \,|\, \boldsymbol{X}, \boldsymbol{\lambda}) \,
 *     p(\boldsymbol{Y} \,|\, \boldsymbol{X}, \hat{\boldsymbol{m}},
 *                            \boldsymbol{\lambda}) \\
 *   &=& \mathop{\mathrm{argmax}}_{\boldsymbol{y}}
 *     \prod_{t=0}^{T-1}
 *     p(\hat{m}_t \,|\, \boldsymbol{X}_t, \boldsymbol{\lambda}) \,
 *     p(\boldsymbol{Y}_t \,|\, \boldsymbol{X}_t, \hat{m}_t,
 *                              \boldsymbol{\lambda}),
 * @f}
 * where
 * @f[
 *   p(\hat{m}_t \,|\, \boldsymbol{X}_t, \boldsymbol{\lambda}) = \max_m \,
 *     \frac{w_m \mathcal{N} \left( \boldsymbol{X}_t \,\big|\,
 *       \boldsymbol{\mu}_m^{(X)}, \boldsymbol{\varSigma}_m^{(XX)} \right)}
 *     {\sum_{n=1}^M w_n \mathcal{N} \left( \boldsymbol{X}_t \,\big|\,
 *       \boldsymbol{\mu}_n^{(X)}, \boldsymbol{\varSigma}_n^{(XX)} \right)},
 * @f]
 * and
 * @f{eqnarray}
 *  p(\boldsymbol{Y}_t \,|\, \boldsymbol{X}_t, m, \boldsymbol{\lambda}) &=&
 *    \mathcal{N} \left( \boldsymbol{Y}_t \,\big|\, \boldsymbol{E}_{m,t}^{(Y)},
 *                \boldsymbol{D}_{m,t}^{(Y)} \right), \\
 *  \boldsymbol{E}_{m,t}^{(Y)} &=&
 *    \boldsymbol{\mu}_m^{(Y)} + \boldsymbol{\varSigma}_m^{(YX)}
 *    \boldsymbol{\varSigma}_m^{(XX)^{-1}}
 *    \left( \boldsymbol{X}_t - \boldsymbol{\mu}_m^{(X)} \right), \\
 *  \boldsymbol{D}_{m,t}^{(Y)} &=&
 *    \boldsymbol{\varSigma}_m^{(YY)} - \boldsymbol{\varSigma}_m^{(YX)}
 *    \boldsymbol{\varSigma}_m^{(XX)^{-1}} \boldsymbol{\varSigma}_m^{(XY)}.
 * @f}
 * The converted static vector sequence @f$\hat{\boldsymbol{y}}@f$ under the
 * constraint between static and dynamic components is obtained by the maximum
 * likelihood parameter generation algorithm.
 *
 * [1] T. Toda, A. W. Black, and K. Tokuda, &quot;Voice conversion based on
 *     maximum-likelihood estimation of spectral parameter trajectory,&quot;
 *     IEEE Transactions on Audio, Speech, and Language Processing, vol. 15,
 *     no. 8, pp. 2222-2235, 2007.
 */
class GaussianMixtureModelBasedConversion {
 public:
  /**
   * @param[in] num_source_order Order of source vector, @f$M_1@f$.
   * @param[in] num_target_order Order of target vector, @f$M_2@f$.
   * @param[in] window_coefficients Window coefficients.
   *            e.g.) { {-0.5, 0.0, 0.5}, {1.0, -2.0, 1.0} }
   * @param[in] weights @f$K@f$ mixture weights.
   * @param[in] mean_vectors @f$K@f$ mean vectors.
   *            The shape is @f$[K, (D+1)(M_1+M_2+2)]@f$.
   * @param[in] covariance_matrices @f$K@f$ covariance matrices.
   *            The shape is @f$[K, (D+1)(M_1+M_2+2), (D+1)(M_1+M_2+2)]@f$.
   * @param[in] use_magic_number Whether to use magic number.
   * @param[in] magic_number A magic number represents a discrete symbol.
   */
  GaussianMixtureModelBasedConversion(
      int num_source_order, int num_target_order,
      const std::vector<std::vector<double> >& window_coefficients,
      const std::vector<double>& weights,
      const std::vector<std::vector<double> >& mean_vectors,
      const std::vector<SymmetricMatrix>& covariance_matrices,
      bool use_magic_number, double magic_number = 0.0);

  virtual ~GaussianMixtureModelBasedConversion() {
  }

  /**
   * @return Order of source vector.
   */
  int GetNumSourceOrder() const {
    return num_source_order_;
  }

  /**
   * @return Order of target vector.
   */
  int GetNumTargetOrder() const {
    return num_target_order_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] source_vectors @f$M_1@f$-th order source vectors containing
   *            dynamic components. The shape is @f$[T, (D+1)(M_1+1)]@f$.
   * @param[out] target_vectors @f$M_2@f$-th order target vectors.
   *             The shape is @f$[T, (M_2+1)]@f$.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<std::vector<double> >& source_vectors,
           std::vector<std::vector<double> >* target_vectors) const;

 private:
  const int num_source_order_;
  const int num_target_order_;
  const int source_length_;
  const int target_length_;

  const std::vector<double> weights_;
  const bool use_magic_number_;
  const double magic_number_;

  const int num_mixture_;
  const NonrecursiveMaximumLikelihoodParameterGeneration mlpg_;

  bool is_valid_;

  std::vector<std::vector<double> > source_mean_vectors_;
  std::vector<SymmetricMatrix> source_covariance_matrices_;
  std::vector<Matrix> e_slope_;
  std::vector<std::vector<double> > e_bias_;
  std::vector<SymmetricMatrix> d_;

  DISALLOW_COPY_AND_ASSIGN(GaussianMixtureModelBasedConversion);
};

}  // namespace sptk

#endif  // SPTK_MATH_GAUSSIAN_MIXTURE_MODEL_BASED_CONVERSION_H_
