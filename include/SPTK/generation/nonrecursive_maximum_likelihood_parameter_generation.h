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
   * @param[in] mean_vectors Mean vector sequence. The shape is @f$[T, DM]$@f$.
   * @param[in] variance_vectors Variance vector sequence.
   *            The shape is @f$[T, DM]$@f$.
   * @param[out] smoothed_static_parameters Smoothed static parameters.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<std::vector<double> >& mean_vectors,
           const std::vector<std::vector<double> >& variance_vectors,
           std::vector<std::vector<double> >* smoothed_static_parameters);

  /**
   * @param[in] mean_vectors Mean vector sequence. The shape is @f$[T, DM]$@f$.
   * @param[in] covariance_matrices Covariance matrix sequence.
   *            The shape is @f$[T, DM, DM]$@f$.
   * @param[out] smoothed_static_parameters Smoothed static parameters.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<std::vector<double> >& mean_vectors,
           const std::vector<sptk::SymmetricMatrix>& covariance_matrices,
           std::vector<std::vector<double> >* smoothed_static_parameters);

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
