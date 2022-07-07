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

#ifndef SPTK_GENERATION_RECURSIVE_MAXIMUM_LIKELIHOOD_PARAMETER_GENERATION_H_
#define SPTK_GENERATION_RECURSIVE_MAXIMUM_LIKELIHOOD_PARAMETER_GENERATION_H_

#include <vector>  // std::vector

#include "SPTK/input/input_source_interface.h"
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
 *
 * The impelemented algorithm is based on a recursive manner using Kalman
 * filter. The algorithm does not require entire mean and varaiance parameter
 * sequence, but intorduces approximation error. The amount of approximation
 * error is controlled by a parameter, @f$S@f$.
 */
class RecursiveMaximumLikelihoodParameterGeneration
    : public InputSourceInterface {
 public:
  /**
   * @param[in] num_order Order of coefficients, @f$M@f$.
   * @param[in] num_past_frame Number of past frames, @f$S@f$.
   * @param[in] window_coefficients Window coefficients.
   *            e.g.) { {-0.5, 0.0, 0.5}, {1.0, -2.0, 1.0} }
   * @param[in] input_source Static and dynamic components sequence.
   */
  RecursiveMaximumLikelihoodParameterGeneration(
      int num_order, int num_past_frame,
      const std::vector<std::vector<double> >& window_coefficients,
      InputSourceInterface* input_source);

  virtual ~RecursiveMaximumLikelihoodParameterGeneration() {
  }

  /**
   * @return Order of coefficients.
   */
  int GetNumOrder() const {
    return num_order_;
  }

  /**
   * @return Number of past frames.
   */
  int GetNumPastFrame() const {
    return num_past_frame_;
  }

  /**
   * @return Output size.
   */
  virtual int GetSize() const {
    return num_order_ + 1;
  }

  /**
   * @return True if this object is valid.
   */
  virtual bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[out] smoothed_static_parameters Smoothed static parameters.
   * @return True on success, false on failure.
   */
  virtual bool Get(std::vector<double>* smoothed_static_parameters);

 private:
  struct Buffer {
    std::vector<double> static_and_dynamic_parameters;
    std::vector<std::vector<double> > stored_dynamic_mean_vectors;
    std::vector<std::vector<double> >
        stored_dynamic_diagonal_covariance_matrices;
    std::vector<std::vector<double> > pi;
    std::vector<std::vector<double> > k;
    std::vector<std::vector<std::vector<double> > > p;
    std::vector<std::vector<double> > c;
  };

  bool Forward();

  const int num_order_;
  const int num_past_frame_;
  std::vector<std::vector<double> > window_coefficients_;
  InputSourceInterface* input_source_;

  bool is_valid_;

  int calculation_field_;
  int num_remaining_frame_;
  int current_frame_;

  Buffer buffer_;

  DISALLOW_COPY_AND_ASSIGN(RecursiveMaximumLikelihoodParameterGeneration);
};

}  // namespace sptk

#endif  // SPTK_GENERATION_RECURSIVE_MAXIMUM_LIKELIHOOD_PARAMETER_GENERATION_H_
