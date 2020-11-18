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

#ifndef SPTK_CONVERSION_FILTER_COEFFICIENTS_TO_PHASE_SPECTRUM_H_
#define SPTK_CONVERSION_FILTER_COEFFICIENTS_TO_PHASE_SPECTRUM_H_

#include <vector>  // std::vector

#include "SPTK/math/real_valued_fast_fourier_transform.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Transform filter coefficients to phase spectrum.
 *
 * The input is the @f$M@f$-th order numerator coefficients and the @f$N@f$-th
 * order denominator coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     b(0), & b(1), & \ldots, & b(M), \\
 *     K,    & a(1), & \ldots, & a(N),
 *   \end{array}
 * @f]
 * and the output is the @f$(L/2+1)@f$-length phase spectrum:
 * @f[
 *   \begin{array}{cccc}
 *     \angle H(0), & \angle H(1), & \ldots, & \angle H(L/2),
 *   \end{array}
 * @f]
 * where @f$L@f$ is the FFT length.
 *
 * The general form of transfer function is given by
 * @f{eqnarray}{
 *   H(z) &=& \frac{\sum_{m=0}^M b(m) z^{-m}}{\sum_{n=0}^N a(n) z^{-n}} \\
 *        &=& \frac{B(z)}{A(z)}.
 * @f}
 * where @f$a(0)=1@f$. It can be rewritten as
 * @f{eqnarray}{
 *   H(z) &=& \frac{B_R(z) + i B_I(z)}{A_R(z) + i A_I(z)} \\
 *        &=& \frac{B_R(z) + i B_I(z)}{A_R(z) + i A_I(z)} \cdot
 *            \frac{A_R(z) - i A_I(z)}{A_R(z) - i A_I(z)} \\
 *        &=& \frac{B_R(z) A_R(z) + B_I(z) A_I(z)}{A_R^2(z) + A_I^2(z)} +i
 *            \frac{B_I(z) A_R(z) - B_R(z) A_I(z)}{A_R^2(z) + A_I^2(z)}.
 * @f}
 * where the subscripts @f$R@f$ and @f$I@f$ denote the real and imaginary parts.
 * Thus
 * @f{eqnarray}{
 *   \angle H(z) &=& \tan^{-1} \left(\frac{H_I(z)}{H_R(z)}\right) \\
 *               &=& \tan^{-1} \left( \frac{B_I(z) A_R(z) - B_R(z) A_I(z)}
 *                             {B_R(z) A_R(z) + B_I(z) A_I(z)} \right).
 * @f}
 */
class FilterCoefficientsToPhaseSpectrum {
 public:
  /**
   * Buffer for FilterCoefficientsToPhaseSpectrum class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    RealValuedFastFourierTransform::Buffer buffer_for_fast_fourier_transform_;

    std::vector<double> real_part1_;
    std::vector<double> real_part2_;
    std::vector<double> imag_part1_;
    std::vector<double> imag_part2_;

    friend class FilterCoefficientsToPhaseSpectrum;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_numerator_order Order of numerator coefficients, @f$M@f$.
   * @param[in] num_denominator_order Order of denominator coefficients,
   *            @f$N@f$.
   * @param[in] fft_length Number of FFT bins, @f$L@f$.
   * @param[in] unwrapping If true, perform phase unwrapping.
   */
  FilterCoefficientsToPhaseSpectrum(int num_numerator_order,
                                    int num_denominator_order, int fft_length,
                                    bool unwrapping);

  virtual ~FilterCoefficientsToPhaseSpectrum() {
  }

  /**
   * @return Order of numerator coefficients.
   */
  int GetNumNumeratorOrder() const {
    return num_numerator_order_;
  }

  /**
   * @return Order of denominator coefficients.
   */
  int GetNumDenominatorOrder() const {
    return num_denominator_order_;
  }

  /**
   * @return FFT length.
   */
  int GetFftLength() const {
    return fft_length_;
  }

  /**
   * @return True if unwrapping is performed.
   */
  bool IsUnwrapped() const {
    return unwrapping_;
  }

  /**
   * @return True if this obejct is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] numerator_coefficients @f$M@f$-th order coefficients.
   * @param[in] denominator_coefficients @f$N@f$-th order coefficients.
   * @param[out] phase_spectrum @f$(L/2+1)@f$-length phase spectrum.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& numerator_coefficients,
           const std::vector<double>& denominator_coefficients,
           std::vector<double>* phase_spectrum,
           FilterCoefficientsToPhaseSpectrum::Buffer* buffer) const;

 private:
  const int num_numerator_order_;
  const int num_denominator_order_;
  const int fft_length_;
  const bool unwrapping_;

  const RealValuedFastFourierTransform fast_fourier_transform_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(FilterCoefficientsToPhaseSpectrum);
};

}  // namespace sptk

#endif  // SPTK_CONVERSION_FILTER_COEFFICIENTS_TO_PHASE_SPECTRUM_H_
