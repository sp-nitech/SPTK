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

#ifndef SPTK_CHECK_LINE_SPECTRAL_PAIRS_STABILITY_CHECK_H_
#define SPTK_CHECK_LINE_SPECTRAL_PAIRS_STABILITY_CHECK_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Check stability of line spectral pairs and modify them.
 *
 * The input is the @f$M@f$-th order LSP:
 * @f[
 *   \begin{array}{cccc}
 *     K, & \omega(1), & \ldots, & \omega(M),
 *   \end{array}
 * @f]
 * and the output is the modified @f$M@f$-th order LSP:
 * @f[
 *   \begin{array}{cccc}
 *     K, & \omega'(1), & \ldots, & \omega'(M).
 *   \end{array}
 * @f]
 * The stability of LSP can be written as
 * @f{eqnarray}{
 *                   \omega(1) &\ge& \delta, \\
 *   \omega(m) - \omega(m - 1) &\ge& \delta, \quad (1 < m < M) \\
 *                   \omega(M) &\le& \pi - \delta, \\
 * @f}
 * where @f$0 \le \delta \le \pi/(M + 1)@f$.
 */
class LineSpectralPairsStabilityCheck {
 public:
  /**
   * @param[in] num_order Order of LSP, @f$M@f$.
   * @param[in] minimum_distance Minimum distance between lines, @f$\delta@f$.
   */
  LineSpectralPairsStabilityCheck(int num_order, double minimum_distance);

  virtual ~LineSpectralPairsStabilityCheck() {
  }

  /**
   * @return Order of coefficients.
   */
  int GetNumOrder() const {
    return num_order_;
  }

  /**
   * @return Minimum distance between lines.
   */
  double GetMinimumDistance() const {
    return minimum_distance_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] line_spectral_pairs @f$M@f$-th order LSP.
   * @param[out] modified_line_spectral_pairs Modified @f$M@f$-th order LSP
   *             (optional).
   * @param[out] is_stable True if the given coefficients are stable.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& line_spectral_pairs,
           std::vector<double>* modified_line_spectral_pairs,
           bool* is_stable) const;

  /**
   * @param[in,out] input_and_output @f$M@f$-th order coefficients.
   * @param[out] is_stable True if the given coefficients are stable.
   * @return True on success, false on failure.
   */
  bool Run(std::vector<double>* input_and_output, bool* is_stable) const;

 private:
  const int num_order_;
  const double minimum_distance_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(LineSpectralPairsStabilityCheck);
};

}  // namespace sptk

#endif  // SPTK_CHECK_LINE_SPECTRAL_PAIRS_STABILITY_CHECK_H_
