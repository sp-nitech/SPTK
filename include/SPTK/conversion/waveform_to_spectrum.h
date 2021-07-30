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
 *     20 \log_{10}|H(0)|, & 20 \log_{10}|H(1)|, & \ldots, & 20
 * \log_{10}|H(N/2)|. \end{array}
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
