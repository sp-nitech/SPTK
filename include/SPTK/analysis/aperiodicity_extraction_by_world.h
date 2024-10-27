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

#ifndef SPTK_ANALYSIS_APERIODICITY_EXTRACTION_BY_WORLD_H_
#define SPTK_ANALYSIS_APERIODICITY_EXTRACTION_BY_WORLD_H_

#include <vector>  // std::vector

#include "SPTK/analysis/aperiodicity_extraction_interface.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Extract aperiodicity based on WORLD (D4C).
 */
class AperiodicityExtractionByWorld : public AperiodicityExtractionInterface {
 public:
  /**
   * @param[in] fft_length FFT length.
   * @param[in] frame_shift Frame shift in point.
   * @param[in] sampling_rate Sampling rate in Hz.
   */
  AperiodicityExtractionByWorld(int fft_length, int frame_shift,
                                double sampling_rate);

  ~AperiodicityExtractionByWorld() override {
  }

  /**
   * @return FFT length.
   */
  int GetFftLength() const {
    return fft_length_;
  }

  /**
   * @return Frame shift.
   */
  int GetFrameShift() const override {
    return frame_shift_;
  }

  /**
   * @return Sampling rate.
   */
  double GetSamplingRate() const {
    return sampling_rate_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const override {
    return is_valid_;
  }

  /**
   * @param[in] waveform Waveform.
   * @param[in] f0 Fundamental frequency in Hz.
   * @param[out] aperiodicity Aperiodicity measure.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& waveform, const std::vector<double>& f0,
           std::vector<std::vector<double> >* aperiodicity) const override;

 private:
  const int fft_length_;
  const int frame_shift_;
  const double sampling_rate_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(AperiodicityExtractionByWorld);
};

}  // namespace sptk

#endif  // SPTK_ANALYSIS_APERIODICITY_EXTRACTION_BY_WORLD_H_
