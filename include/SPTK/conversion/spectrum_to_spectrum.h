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
