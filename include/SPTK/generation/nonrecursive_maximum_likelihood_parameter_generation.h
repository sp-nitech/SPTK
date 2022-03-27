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

#ifndef SPTK_GENERATION_NONRECURSIVE_MAXIMUM_LIKELIHOOD_PARAMETER_GENERATION_H_
#define SPTK_GENERATION_NONRECURSIVE_MAXIMUM_LIKELIHOOD_PARAMETER_GENERATION_H_

#include <vector>  // std::vector

#include "SPTK/math/symmetric_matrix.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Estimate the maximum likelihood parameters from the means and the diagonal
 * covariances of Gaussian distributions.
 *
 * The input is the sequence of the mean and the diagnoal covariance of
 * @f$M@f$-th order static and @f$DM@f$-th order dynamic feature components:
 * @f[
 *   \begin{array}{ccccc}
 *     \boldsymbol{\mu}_1, & \boldsymbol{\varSigma}_1, & \ldots, &
 *     \boldsymbol{\mu}_T, & \boldsymbol{\varSigma}_T,
 *   \end{array}
 * @f]
 * where
 * @f{eqnarray}{
 *   \boldsymbol{\mu}_t &=& \left[ \begin{array}{cccc}
 *     \boldsymbol{\mu}_t^{(0)}, & \boldsymbol{\mu}_t^{(1)}, &
 *     \ldots, & \boldsymbol{\mu}_t^{(D)}
 *   \end{array} \right] \\
 *   \boldsymbol{\varSigma}_t &=& \left[ \begin{array}{cccc}
 *     \boldsymbol{\varSigma}_t^{(0)}, & \boldsymbol{\varSigma}_t^{(1)}, &
 *     \ldots, & \boldsymbol{\varSigma}_t^{(D)}
 *   \end{array} \right].
 * @f}
 * The output is the sequence of the @f$M@f$-th order smoothed static feature
 * components:
 * @f[
 *   \begin{array}{cccc}
 *     \boldsymbol{c}_1, & \boldsymbol{c}_2, & \ldots, &
 *     \boldsymbol{c}_T.
 *   \end{array}
 * @f]
 */
class NonrecursiveMaximumLikelihoodParameterGeneration {
 public:
  /**
   * @param[in] num_order Order of coefficients, @f$M@f$.
   * @param[in] window_coefficients Window coefficients.
   *            e.g.) { {-0.5, 0.0, 0.5}, {1.0, -2.0, 1.0} }
   * @param[in] use_magic_number Whether to use magic number.
   * @param[in] magic_number A magic number represents a discrete symbol.
   */
  NonrecursiveMaximumLikelihoodParameterGeneration(
      int num_order,
      const std::vector<std::vector<double> >& window_coefficients,
      bool use_magic_number, double magic_number = 0.0);

  virtual ~NonrecursiveMaximumLikelihoodParameterGeneration() {
  }

  /**
   * @return Order of coefficients.
   */
  int GetNumOrder() const {
    return num_order_;
  }

  /**
   * @return Magic number.
   */
  double GetMagicNumber() const {
    return magic_number_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] mean_vectors Mean vector sequence. The shape is @f$[T, DM]@f$.
   * @param[in] variance_vectors Variance vector sequence.
   *            The shape is @f$[T, DM]@f$.
   * @param[out] smoothed_static_parameters Smoothed static parameters.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<std::vector<double> >& mean_vectors,
           const std::vector<std::vector<double> >& variance_vectors,
           std::vector<std::vector<double> >* smoothed_static_parameters) const;

  /**
   * @param[in] mean_vectors Mean vector sequence. The shape is @f$[T, DM]@f$.
   * @param[in] covariance_matrices Covariance matrix sequence.
   *            The shape is @f$[T, DM, DM]@f$.
   * @param[out] smoothed_static_parameters Smoothed static parameters.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<std::vector<double> >& mean_vectors,
           const std::vector<SymmetricMatrix>& covariance_matrices,
           std::vector<std::vector<double> >* smoothed_static_parameters) const;

 private:
  const int num_order_;
  std::vector<std::vector<double> > window_coefficients_;
  const bool use_magic_number_;
  const double magic_number_;

  bool is_valid_;
  int max_half_window_width_;

  DISALLOW_COPY_AND_ASSIGN(NonrecursiveMaximumLikelihoodParameterGeneration);
};

}  // namespace sptk

#endif  // SPTK_GENERATION_NONRECURSIVE_MAXIMUM_LIKELIHOOD_PARAMETER_GENERATION_H_
