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

#ifndef SPTK_UTILS_MISC_UTILS_H_
#define SPTK_UTILS_MISC_UTILS_H_

#include <vector>  // std::vector

namespace sptk {

/**
 * Compute the percentage point of the standard normal distribution using the
 * formula for the approximation by Toda and Ono.
 *
 * [1] H. Toda and H. Ono, &quot;The minimax approximation for percentage points
 *     of the standard normal distribution,&quot; Japanese journal of applied
 *     statistics, vol. 22, no. 1, pp. 13-21, 1993.
 *
 * @param[in] probability Probability.
 * @param[out] percentage_point Percentage point.
 * @return True on success, false on failure.
 */
bool ComputePercentagePointOfStandardNormalDistribution(
    double probability, double* percentage_point);

/**
 * Compute the probablity of the t-distribution.
 *
 * @param[in] percentage_point Percentage point.
 * @param[in] degrees_of_freedom Degrees of freedom.
 * @param[out] probability Probability.
 * @return True on success, false on failure.
 */
bool ComputeProbabilityOfTDistribution(double percentage_point,
                                       int degrees_of_freedom,
                                       double* probability);

/**
 * Compute the percentage point of the t-distribution using the Cornish-Fisher
 * expansion in the percentage point of the standard normal distribution.
 *
 * [1] R. A. Fisher and E. A. Cornish, &quot;The percentile points of
 *     distributions having known cumulants,&quot; Technometrics, vol. 2, no. 2,
 *     pp. 209-225, 1960.
 *
 * @param[in] probability Probability.
 * @param[in] degrees_of_freedom Degrees of freedom.
 * @param[out] percentage_point Percentage point.
 * @return True on success, false on failure.
 */
bool ComputePercentagePointOfTDistribution(double probability,
                                           int degrees_of_freedom,
                                           double* percentage_point);

/**
 * Make pseudo quadrature mirror filter banks under given condition.
 *
 * [1] T. Q. Nguyen, &quot;Near-perfect-reconstruction pseudo-QMF banks,&quot;
 *     IEEE Transactions on Signal Processing, vol. 42, no. 1, pp. 65-76, 1994.
 *
 * [2] Y.-P. Lin and P. P. Vaidyanathan, &quot;A Kaiser window approach for the
 *     design of prototype filters of cosine modulated filterbanks,&quot; IEEE
 *     Signal Processing Letters, vol. 5, no. 6, pp. 132-134, 1998.
 *
 * [3] F. Cruz-Roldan, P. Amo-Lopez, S. Maldonado-Bascon, and S. S. Lawson,
 *     &quot;An efficient and simple method for designing prototype filters for
 *     cosine-modulated pseudo-QMF banks,&quot; IEEE Signal Processing Letters,
 *     vol. 9, no. 1, pp. 29-31, 2002.
 *
 * @param[in] inverse If true, filter banks for synthesis is calculated.
 * @param[in] num_subband Number of subbands.
 * @param[in] num_order Order of filter.
 * @param[in] attenuation Stopband attenuation.
 * @param[in] num_iteration Number of iterations.
 * @param[in] convergence_threshold Convergence threshold.
 * @param[in] initial_step_size Initial step size.
 * @param[out] filter_banks Filter banks.
 * @param[out] is_converged True if convergence is reached (optional).
 * @return True on success, false on failure.
 */
bool MakePseudoQuadratureMirrorFilterBanks(
    bool inverse, int num_subband, int num_order, double attenuation,
    int num_iteration, double convergence_threshold, double initial_step_size,
    std::vector<std::vector<double> >* filter_banks, bool* is_converged);

/**
 * Perform 1D convolution.
 *
 * @param[in] f First input signal.
 * @param[in] g Second Input signal.
 * @param[out] result Output signal.
 * @return True on success, false on failure.
 */
bool Perform1DConvolution(const std::vector<double>& f,
                          const std::vector<double>& g,
                          std::vector<double>* result);

/**
 * Compute 1st order regression coefficients.
 *
 * @param[in] n Width of regression coefficients.
 * @param[out] coefficients Regression coefficients.
 * @return True on success, false on failure.
 */
bool ComputeFirstOrderRegressionCoefficients(int n,
                                             std::vector<double>* coefficients);

/**
 * Compute 2nd order regression coefficients.
 *
 * @param[in] n Width of regression coefficients.
 * @param[out] coefficients Regression coefficients.
 * @return True on success, false on failure.
 */
bool ComputeSecondOrderRegressionCoefficients(
    int n, std::vector<double>* coefficients);

}  // namespace sptk

#endif  // SPTK_UTILS_MISC_UTILS_H_
