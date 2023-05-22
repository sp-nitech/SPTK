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

#ifndef SPTK_FILTER_WORLD_SYNTHESIS_H_
#define SPTK_FILTER_WORLD_SYNTHESIS_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Synthesize speech waveform using WORLD vocoder.
 */
class WorldSynthesis {
 public:
  /**
   * @param[in] fft_length FFT length.
   * @param[in] frame_shift Frame shift in point.
   * @param[in] sampling_rate Sampling rate in Hz.
   */
  WorldSynthesis(int fft_length, int frame_shift, double sampling_rate);

  virtual ~WorldSynthesis() {
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
  virtual int GetFrameShift() const {
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
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] f0 Fundamental frequency in Hz.
   * @param[in] spectrum Power spectrum.
   * @param[in] aperiodicity Aperiodicity ratio.
   * @param[out] waveform Synthesized waveform.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& f0,
           const std::vector<std::vector<double>>& spectrum,
           const std::vector<std::vector<double>>& aperiodicity,
           std::vector<double>* waveform) const;

 private:
  const int fft_length_;
  const int frame_shift_;
  const double sampling_rate_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(WorldSynthesis);
};

}  // namespace sptk

#endif  // SPTK_FILTER_WORLD_SYNTHESIS_H_
