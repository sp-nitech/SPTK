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
