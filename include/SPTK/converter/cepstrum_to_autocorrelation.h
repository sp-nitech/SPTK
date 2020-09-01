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

#ifndef SPTK_CONVERTER_CEPSTRUM_TO_AUTOCORRELATION_H_
#define SPTK_CONVERTER_CEPSTRUM_TO_AUTOCORRELATION_H_

#include <vector>  // std::vector

#include "SPTK/math/real_valued_fast_fourier_transform.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Transform cepstral coefficients to autocorrelation coefficients.
 *
 * The input is the \f$M_1\f$-th order cepstral coefficients:
 * \f[
 *   \begin{array}{cccc}
 *     c(0), & c(1), & \ldots, & c(M_1),
 *   \end{array}
 * \f]
 * and the output is the \f$M_2\f$-th order autocorrelation coefficients:
 * \f[
 *   \begin{array}{cccc}
 *     r(0), & r(1), & \ldots, & r(M_2),
 *   \end{array}
 * \f]
 * The definition of the cepstrum can be represented as
 * \f[
 *   c(m) = \mathcal{F}^{-1} \{ \log |\mathcal{F} \{x(m)\}| \},
 * \f]
 * where \f$x(m)\f$ is a signal, \f$\mathcal{F}^{-1}\f$ and \f$\mathcal{F}\f$
 * denote the DFT and the inverse DFT, respectively. From the definition, the
 * relation between the cepstrum and the autocorrelation can be derived as
 * follows:
 * \f{eqnarray}{
 *    c(m) &=& \mathcal{F}^{-1} \left\{
 *             \frac{1}{2} \log|X(\omega)|^2 \right\} \\
 *         &=& \mathcal{F}^{-1} \left\{
 *             \frac{1}{2} \log \mathcal{F} \{r(m)\} \right\},
 * \f}
 * where the Wiener–Khinchin theorem is used. Thus
 * \f[
 *    r(m) = \mathcal{F}^{-1} \{ \exp (2\mathcal{F} \{c(m)\}) \}.
 * \f]
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
   * @param[in] num_input_order Order of cepstral coefficients, \f$M_1\f$.
   * @param[in] num_output_order Order of autocorrelation coefficients,
   *            \f$M_2\f$.
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
   * @return True if this obejct is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] cepstrum \f$M_1\f$-th order cesptral coefficients.
   * @param[out] autocorrelation \f$M_2\f$-th order autocorrelation
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

#endif  // SPTK_CONVERTER_CEPSTRUM_TO_AUTOCORRELATION_H_
