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

#ifndef SPTK_ANALYSIS_AUTOCORRELATION_ANALYSIS_H_
#define SPTK_ANALYSIS_AUTOCORRELATION_ANALYSIS_H_

#include <vector>  // std::vector

#include "SPTK/conversion/spectrum_to_autocorrelation.h"
#include "SPTK/conversion/waveform_to_autocorrelation.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Calculate autocorrelation from waveform or spectrum.
 */
class AutocorrelationAnalysis {
 public:
  /**
   * Buffer for AutocorrelationAnalysis class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    SpectrumToAutocorrelation::Buffer buffer_;

    friend class AutocorrelationAnalysis;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] frame_length Frame length or number of FFT bins, @f$L@f$.
   * @param[in] num_order Order of autocorrelation, @f$M@f$.
   * @param[in] waveform_input If true, assume waveform input.
   */
  AutocorrelationAnalysis(int frame_length, int num_order, bool waveform_input);

  virtual ~AutocorrelationAnalysis();

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] input @f$L@f$-length waveform or @f$(L/2+1)@f$-length power
   *            spectrum.
   * @param[out] autocorrelation @f$M@f$-th order autocorrelation coefficients.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& input,
           std::vector<double>* autocorrelation,
           AutocorrelationAnalysis::Buffer* buffer) const;

 private:
  const bool waveform_input_;

  WaveformToAutocorrelation* waveform_to_autocorrelation_;
  SpectrumToAutocorrelation* spectrum_to_autocorrelation_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(AutocorrelationAnalysis);
};

}  // namespace sptk

#endif  // SPTK_ANALYSIS_AUTOCORRELATION_ANALYSIS_H_
