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

#ifndef SPTK_CONVERSION_WAVEFORM_TO_SPECTRUM_H_
#define SPTK_CONVERSION_WAVEFORM_TO_SPECTRUM_H_

#include <vector>  // std::vector

#include "SPTK/conversion/filter_coefficients_to_spectrum.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Convert waveform to spectrum.
 *
 * The input is the @f$L@f$-length waveform signals:
 * @f[
 *   \begin{array}{cccc}
 *     x(0), & x(1), & \ldots, & x(L-1).
 *   \end{array}
 * @f]
 * The output is the @f$(N/2+1)@f$-length spectrum and takes one of the
 * following forms.
 *
 * Type 0:
 * @f[
 *   \begin{array}{cccc}
 *     20 \log_{10}|H(0)|, & 20 \log_{10}|H(1)|, & \ldots, &
 *     20 \log_{10}|H(N/2)|.
 *   \end{array}
 * @f]
 * Type 1:
 * @f[
 *   \begin{array}{cccc}
 *     \ln|H(0)|, & \ln|H(1)|, & \ldots, & \ln|H(N/2)|.
 *   \end{array}
 * @f]
 * Type 2:
 * @f[
 *   \begin{array}{cccc}
 *     |H(0)|, & |H(1)|, & \ldots, & |H(N/2)|.
 *   \end{array}
 * @f]
 * Type 3:
 * @f[
 *   \begin{array}{cccc}
 *     |H(0)|^2, & |H(1)|^2, & \ldots, & |H(N/2)|^2,
 *   \end{array}
 * @f]
 * where @f$N@f$ is the FFT length.
 */
class WaveformToSpectrum {
 public:
  /**
   * Buffer for WaveformToSpectrum class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    FilterCoefficientsToSpectrum::Buffer buffer_;

    friend class WaveformToSpectrum;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] frame_length Frame length, @f$L@f$.
   * @param[in] fft_length FFT length, @f$N@f$.
   * @param[in] output_format Output format.
   * @param[in] epsilon Small value added to power spectrum.
   * @param[in] relative_floor_in_decibels Relative floor in decibels.
   */
  WaveformToSpectrum(int frame_length, int fft_length,
                     SpectrumToSpectrum::InputOutputFormats output_format,
                     double epsilon, double relative_floor_in_decibels);

  virtual ~WaveformToSpectrum() {
  }

  /**
   * @return Frame length.
   */
  int GetFrameLength() const {
    return filter_coefficients_to_spectrum_.GetNumNumeratorOrder() + 1;
  }

  /**
   * @return FFT length.
   */
  int GetFftLength() const {
    return filter_coefficients_to_spectrum_.GetFftLength();
  }

  /**
   * @return Output format.
   */
  SpectrumToSpectrum::InputOutputFormats GetOutputFormat() const {
    return filter_coefficients_to_spectrum_.GetOutputFormat();
  }

  /**
   * @return Epsilon.
   */
  double GetEpsilon() const {
    return filter_coefficients_to_spectrum_.GetEpsilon();
  }

  /**
   * @return Relative floor.
   */
  double GetRelativeFloorInDecibels() const {
    return filter_coefficients_to_spectrum_.GetRelativeFloorInDecibels();
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return filter_coefficients_to_spectrum_.IsValid();
  }

  /**
   * @param[in] waveform @f$L@f$-length waveform signals.
   * @param[out] spectrum @f$(N/2+1)@f$-length spectrum.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& waveform, std::vector<double>* spectrum,
           WaveformToSpectrum::Buffer* buffer) const;

 private:
  const FilterCoefficientsToSpectrum filter_coefficients_to_spectrum_;

  const std::vector<double> dummy_for_filter_coefficients_to_spectrum_;

  DISALLOW_COPY_AND_ASSIGN(WaveformToSpectrum);
};

}  // namespace sptk

#endif  // SPTK_CONVERSION_WAVEFORM_TO_SPECTRUM_H_
