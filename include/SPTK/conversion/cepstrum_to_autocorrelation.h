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

#ifndef SPTK_CONVERSION_CEPSTRUM_TO_AUTOCORRELATION_H_
#define SPTK_CONVERSION_CEPSTRUM_TO_AUTOCORRELATION_H_

#include <vector>  // std::vector

#include "SPTK/math/real_valued_fast_fourier_transform.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Convert cepstral coefficients to autocorrelation coefficients.
 *
 * The input is the @f$M_1@f$-th order cepstral coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     c(0), & c(1), & \ldots, & c(M_1),
 *   \end{array}
 * @f]
 * and the output is the @f$M_2@f$-th order autocorrelation coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     r(0), & r(1), & \ldots, & r(M_2),
 *   \end{array}
 * @f]
 * The definition of the cepstrum can be represented as
 * @f[
 *   c(m) = \mathcal{F}^{-1} \{ \log |\mathcal{F} \{x(m)\}| \},
 * @f]
 * where @f$x(m)@f$ is a signal, @f$\mathcal{F}^{-1}@f$ and @f$\mathcal{F}@f$
 * denote the DFT and the inverse DFT, respectively. From the definition, the
 * relation between the cepstrum and the autocorrelation can be derived as
 * follows:
 * @f{eqnarray}{
 *    c(m) &=& \mathcal{F}^{-1} \left\{
 *             \frac{1}{2} \log|X(\omega)|^2 \right\} \\
 *         &=& \mathcal{F}^{-1} \left\{
 *             \frac{1}{2} \log \mathcal{F} \{r(m)\} \right\},
 * @f}
 * where the Wiener–Khinchin theorem is used. Thus
 * @f[
 *    r(m) = \mathcal{F}^{-1} \{ \exp (2\mathcal{F} \{c(m)\}) \}.
 * @f]
 * Note that the imaginary part is zero.
 */
class CepstrumToAutocorrelation {
 public:
  /**
   * Buffer for CepstrumToAutocorrelation class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    std::vector<double> real_part_;
    std::vector<double> imag_part_;
    RealValuedFastFourierTransform::Buffer buffer_for_fast_fourier_transform_;

    friend class CepstrumToAutocorrelation;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_input_order Order of cepstral coefficients, @f$M_1@f$.
   * @param[in] num_output_order Order of autocorrelation coefficients,
   *            @f$M_2@f$.
   * @param[in] fft_length FFT length.
   */
  CepstrumToAutocorrelation(int num_input_order, int num_output_order,
                            int fft_length);

  virtual ~CepstrumToAutocorrelation() {
  }

  /**
   * @return Order of cepstral coefficients.
   */
  int GetNumInputOrder() const {
    return num_input_order_;
  }

  /**
   * @return Order of autocorrelation coefficients.
   */
  int GetNumOutputOrder() const {
    return num_output_order_;
  }

  /**
   * @return FFT length.
   */
  int GetFftLength() const {
    return fast_fourier_transform_.GetFftLength();
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] cepstrum @f$M_1@f$-th order cesptral coefficients.
   * @param[out] autocorrelation @f$M_2@f$-th order autocorrelation
   *             coefficients.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& cepstrum,
           std::vector<double>* autocorrelation,
           CepstrumToAutocorrelation::Buffer* buffer) const;

 private:
  const int num_input_order_;
  const int num_output_order_;
  const RealValuedFastFourierTransform fast_fourier_transform_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(CepstrumToAutocorrelation);
};

}  // namespace sptk

#endif  // SPTK_CONVERSION_CEPSTRUM_TO_AUTOCORRELATION_H_
