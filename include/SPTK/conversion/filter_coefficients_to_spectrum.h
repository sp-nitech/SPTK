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

#ifndef SPTK_CONVERSION_FILTER_COEFFICIENTS_TO_SPECTRUM_H_
#define SPTK_CONVERSION_FILTER_COEFFICIENTS_TO_SPECTRUM_H_

#include <vector>  // std::vector

#include "SPTK/conversion/spectrum_to_spectrum.h"
#include "SPTK/math/real_valued_fast_fourier_transform.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Convert filter coefficients to spectrum.
 *
 * The input is the @f$M@f$-th order numerator coefficients and the @f$N@f$-th
 * order denominator coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     b(0), & b(1), & \ldots, & b(M), \\
 *     K,    & a(1), & \ldots, & a(N).
 *   \end{array}
 * @f]
 * The output is the @f$(L/2+1)@f$-length spectrum and takes one of the
 * following forms.
 *
 * Type 0:
 * @f[
 *   \begin{array}{cccc}
 *     20 \log_{10}|H(0)|, & 20 \log_{10}|H(1)|, & \ldots, &
 *     20 \log_{10}|H(L/2)|.
 *   \end{array}
 * @f]
 * Type 1:
 * @f[
 *   \begin{array}{cccc}
 *     \ln|H(0)|, & \ln|H(1)|, & \ldots, & \ln|H(L/2)|.
 *   \end{array}
 * @f]
 * Type 2:
 * @f[
 *   \begin{array}{cccc}
 *     |H(0)|, & |H(1)|, & \ldots, & |H(L/2)|.
 *   \end{array}
 * @f]
 * Type 3:
 * @f[
 *   \begin{array}{cccc}
 *     |H(0)|^2, & |H(1)|^2, & \ldots, & |H(L/2)|^2,
 *   \end{array}
 * @f]
 * where @f$L@f$ is the FFT length.
 */
class FilterCoefficientsToSpectrum {
 public:
  /**
   * Buffer for FilterCoefficientsToSpectrum class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    RealValuedFastFourierTransform::Buffer fast_fourier_transform_buffer_;

    std::vector<double> fast_fourier_transform_input_;
    std::vector<double> fast_fourier_transform_real_output_;
    std::vector<double> fast_fourier_transform_imaginary_output_;
    std::vector<double> numerator_of_transfer_function_;
    std::vector<double> denominator_of_transfer_function_;

    friend class FilterCoefficientsToSpectrum;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_numerator_order Order of numerator coefficients, @f$M@f$.
   * @param[in] num_denominator_order Order of denominator coefficients,
   *            @f$N@f$.
   * @param[in] fft_length FFT length, @f$L@f$.
   * @param[in] output_format Output format.
   * @param[in] epsilon Small value added to power spectrum.
   * @param[in] relative_floor_in_decibels Relative floor in decibels.
   */
  FilterCoefficientsToSpectrum(
      int num_numerator_order, int num_denominator_order, int fft_length,
      SpectrumToSpectrum::InputOutputFormats output_format, double epsilon,
      double relative_floor_in_decibels);

  virtual ~FilterCoefficientsToSpectrum() {
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
   * @return Output format.
   */
  SpectrumToSpectrum::InputOutputFormats GetOutputFormat() const {
    return spectrum_to_spectrum_.GetOutputFormat();
  }

  /**
   * @return Epsilon.
   */
  double GetEpsilon() const {
    return spectrum_to_spectrum_.GetEpsilon();
  }

  /**
   * @return Relative floor.
   */
  double GetRelativeFloorInDecibels() const {
    return spectrum_to_spectrum_.GetRelativeFloorInDecibels();
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] numerator_coefficients @f$M@f$-th order coefficients.
   * @param[in] denominator_coefficients @f$N@f$-th order coefficients.
   * @param[out] spectrum @f$(L/2+1)@f$-length spectrum.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& numerator_coefficients,
           const std::vector<double>& denominator_coefficients,
           std::vector<double>* spectrum,
           FilterCoefficientsToSpectrum::Buffer* buffer) const;

 private:
  const int num_numerator_order_;
  const int num_denominator_order_;
  const int fft_length_;

  const RealValuedFastFourierTransform fast_fourier_transform_;
  const SpectrumToSpectrum spectrum_to_spectrum_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(FilterCoefficientsToSpectrum);
};

}  // namespace sptk

#endif  // SPTK_CONVERSION_FILTER_COEFFICIENTS_TO_SPECTRUM_H_
