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

#ifndef SPTK_FILTER_LINE_SPECTRAL_PAIRS_DIGITAL_FILTER_H_
#define SPTK_FILTER_LINE_SPECTRAL_PAIRS_DIGITAL_FILTER_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Apply line spectral pairs (LSP) digital filter to signals.
 *
 * The transfer function @f$H(z)@f$ of LSP filter is
 * @f[
 *   H(z) = \displaystyle\frac{K}{1 + \frac{P(z) - 1 + Q(z) - 1}{2}},
 * @f]
 * where
 * @f{eqnarray}{
 *   P(z)&=&(1-z^{-1})\prod_{m=2,4,\ldots}^M (1-2\cos\omega(m)z^{-1}+z^{-2}), \\
 *   Q(z)&=&(1+z^{-1})\prod_{m=1,3,\ldots}^M (1-2\cos\omega(m)z^{-1}+z^{-2}).
 * @f}
 *
 * Given the @f$M@f$-th order filter coefficients,
 * @f[
 *   \begin{array}{cccc}
 *     K, & \omega(1), & \ldots, & \omega(M),
 *   \end{array}
 * @f]
 * an output signal is obtained by applying @f$H(z)@f$ to an input signal in
 * time domain.
 *
 * [1] N. Sugamura and F. Itakura, &quot;Speech data compression by LSP speech
 *     analysis-synthesis technique,&quot; Journal of IEICE, vol. J64-A, no. 8,
 *     pp. 599-606, 1981.
 */
class LineSpectralPairsDigitalFilter {
 public:
  /**
   * Buffer for LineSpectralPairsDigitalFilter class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    std::vector<double> d1_;
    std::vector<double> d2_;
    std::vector<double> ab_;

    friend class LineSpectralPairsDigitalFilter;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_filter_order Order of filter coefficients, @f$M@f$.
   */
  explicit LineSpectralPairsDigitalFilter(int num_filter_order);

  virtual ~LineSpectralPairsDigitalFilter() {
  }

  /**
   * @return Order of coefficients.
   */
  int GetNumFilterOrder() const {
    return num_filter_order_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] filter_coefficients @f$M@f$-th order LSP filter coefficients.
   * @param[in] filter_input Input signal.
   * @param[out] filter_output Output signal.
   * @param[in,out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& filter_coefficients, double filter_input,
           double* filter_output,
           LineSpectralPairsDigitalFilter::Buffer* buffer) const;

  /**
   * @param[in] filter_coefficients @f$M@f$-th order LSP filter coefficients.
   * @param[in,out] input_and_output Input/output signal.
   * @param[in,out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& filter_coefficients,
           double* input_and_output,
           LineSpectralPairsDigitalFilter::Buffer* buffer) const;

 private:
  const int num_filter_order_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(LineSpectralPairsDigitalFilter);
};

}  // namespace sptk

#endif  // SPTK_FILTER_LINE_SPECTRAL_PAIRS_DIGITAL_FILTER_H_
