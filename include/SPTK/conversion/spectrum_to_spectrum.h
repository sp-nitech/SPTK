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
//                1996-2021  Nagoya Institute of Technology          //
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

#ifndef SPTK_CONVERSION_SPECTRUM_TO_SPECTRUM_H_
#define SPTK_CONVERSION_SPECTRUM_TO_SPECTRUM_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Convert spectrum to spectrum.
 *
 * The input and output are the @f$(N/2+1)@f$-length spectrum and take one of
 * the following forms.
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
class SpectrumToSpectrum {
 public:
  /**
   * Input and output format.
   */
  enum InputOutputFormats {
    kLogAmplitudeSpectrumInDecibels = 0,
    kLogAmplitudeSpectrum,
    kAmplitudeSpectrum,
    kPowerSpectrum,
    kNumInputOutputFormats
  };

  /**
   * Interface of spectrum operation.
   */
  class OperationInterface {
   public:
    virtual ~OperationInterface() {
    }

    /**
     * @param[in,out] input_and_output Input/output spectrum.
     * @return True on success, false on failure.
     */
    virtual bool Run(std::vector<double>* input_and_output) const = 0;
  };

  /**
   * @param[in] fft_length FFT length, @f$N@f$.
   * @param[in] input_format Input format.
   * @param[in] output_format Output format.
   * @param[in] epsilon Small value added to power spectrum.
   * @param[in] relative_floor_in_decibels Relative floor in decibels.
   */
  SpectrumToSpectrum(int fft_length, InputOutputFormats input_format,
                     InputOutputFormats output_format, double epsilon,
                     double relative_floor_in_decibels);

  virtual ~SpectrumToSpectrum();

  /**
   * @return FFT length.
   */
  int GetFftLength() const {
    return fft_length_;
  }

  /**
   * @return Input format.
   */
  InputOutputFormats GetInputFormat() const {
    return input_format_;
  }

  /**
   * @return Output format.
   */
  InputOutputFormats GetOutputFormat() const {
    return output_format_;
  }

  /**
   * @return Epsilon.
   */
  double GetEpsilon() const {
    return epsilon_;
  }

  /**
   * @return Relative floor.
   */
  double GetRelativeFloorInDecibels() const {
    return relative_floor_in_decibels_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] input @f$(N/2+1)@f$-length spectrum.
   * @param[out] output @f$(N/2+1)@f$-length spectrum.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& input, std::vector<double>* output) const;

  /**
   * @param[in,out] input_and_output @f$(N/2+1)@f$-length spectrum.
   * @return True on success, false on failure.
   */
  bool Run(std::vector<double>* input_and_output) const;

 private:
  const int fft_length_;
  const InputOutputFormats input_format_;
  const InputOutputFormats output_format_;
  const double epsilon_;
  const double relative_floor_in_decibels_;

  std::vector<SpectrumToSpectrum::OperationInterface*> operations_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(SpectrumToSpectrum);
};

}  // namespace sptk

#endif  // SPTK_CONVERSION_SPECTRUM_TO_SPECTRUM_H_
