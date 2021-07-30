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
//                1996-2019  Nagoya Institute of Technology          //
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
 *     20 \log_{10}|H(0)|, & 20 \log_{10}|H(1)|, & \ldots, & 20
 * \log_{10}|H(L/2)|. \end{array}
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
   * @f$N@f$.
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
