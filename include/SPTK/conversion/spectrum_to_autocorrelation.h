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

#ifndef SPTK_CONVERSION_SPECTRUM_TO_AUTOCORRELATION_H_
#define SPTK_CONVERSION_SPECTRUM_TO_AUTOCORRELATION_H_

#include <vector>  // std::vector

#include "SPTK/math/real_valued_inverse_fast_fourier_transform.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Calculate autocorrelation from power spectrum.
 *
 * The input is the half of power spectrum:
 * @f[
 *   \begin{array}{cccc}
 *     |X(0)|^2, & |X(1)|^2, & \ldots, & |X(L/2)|^2,
 *   \end{array}
 * @f]
 * where @f$L@f$ is the FFT length. The output is the @f$M@f$-th order
 * autocorrelation coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     r(0), & r(1), & \ldots, & r(M).
 *   \end{array}
 * @f]
 * The autocorrelation is given by
 * @f[
 *   r(m) = \frac{1}{L} \sum_{l=0}^{L-1} |X(l)|^2 e^{j \frac{2\pi}{L} lm},
 * @f]
 * where @f$m@f$ is the lag.
 */
class SpectrumToAutocorrelation {
 public:
  /**
   * Buffer for SpectrumToAutocorrelation class.
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

    RealValuedInverseFastFourierTransform::Buffer
        buffer_for_inverse_fast_fourier_transform_;

    friend class SpectrumToAutocorrelation;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] fft_length Number of FFT bins, @f$N@f$.
   * @param[in] num_order Order of autocorrelation, @f$M@f$.
   */
  SpectrumToAutocorrelation(int fft_length, int num_order);

  virtual ~SpectrumToAutocorrelation() {
  }

  /**
   * @return FFT length.
   */
  int GetFftLength() const {
    return inverse_fast_fourier_transform_.GetFftLength();
  }

  /**
   * @return Order of autocorrelation.
   */
  int GetNumOrder() const {
    return num_order_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] power_spectrum @f$(L/2+1)@f$-length power spectrum.
   * @param[out] autocorrelation @f$M@f$-th order autocorrelation coefficients.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& power_spectrum,
           std::vector<double>* autocorrelation,
           SpectrumToAutocorrelation::Buffer* buffer) const;

 private:
  const int num_order_;

  const RealValuedInverseFastFourierTransform inverse_fast_fourier_transform_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(SpectrumToAutocorrelation);
};

}  // namespace sptk

#endif  // SPTK_CONVERSION_SPECTRUM_TO_AUTOCORRELATION_H_
