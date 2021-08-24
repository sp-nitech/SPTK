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

#ifndef SPTK_ANALYSIS_PITCH_EXTRACTION_BY_RAPT_H_
#define SPTK_ANALYSIS_PITCH_EXTRACTION_BY_RAPT_H_

#include <vector>  // std::vector

#include "SPTK/analysis/pitch_extraction_interface.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Extract pitch based on RAPT.
 */
class PitchExtractionByRapt : public PitchExtractionInterface {
 public:
  /**
   * @param[in] frame_shift Frame shift in point.
   * @param[in] sampling_rate Sampling rate in Hz.
   * @param[in] lower_f0 Lower bound of F0 in Hz.
   * @param[in] upper_f0 Upper bound of F0 in Hz.
   * @param[in] voicing_threshold Threshold for determining voiced/unvoiced.
   */
  PitchExtractionByRapt(int frame_shift, double sampling_rate, double lower_f0,
                        double upper_f0, double voicing_threshold);

  virtual ~PitchExtractionByRapt() {
  }

  /**
   * @return Frame shift.
   */
  int GetFrameShift() const {
    return frame_shift_;
  }

  /**
   * @return Sampling rate.
   */
  double GetSamplingRate() const {
    return sampling_rate_;
  }

  /**
   * @return Minimum fundamental frequency to search for.
   */
  double GetLowerF0() const {
    return lower_f0_;
  }

  /**
   * @return Maximum fundamental frequency to search for.
   */
  double GetUpperF0() const {
    return upper_f0_;
  }

  /**
   * @return Voicing threshold.
   */
  double GetVoicingThreshold() const {
    return voicing_threshold_;
  }

  /**
   * @return True if this object is valid.
   */
  virtual bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] waveform Waveform.
   * @param[out] f0 Extracted pitch in Hz.
   * @param[out] epochs Pitchmark (not used).
   * @param[out] polarity Polarity (not used).
   */
  virtual bool Get(const std::vector<double>& waveform, std::vector<double>* f0,
                   std::vector<double>* epochs,
                   PitchExtractionInterface::Polarity* polarity) const;

 private:
  const int frame_shift_;
  const double sampling_rate_;
  const double lower_f0_;
  const double upper_f0_;
  const double voicing_threshold_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(PitchExtractionByRapt);
};

}  // namespace sptk

#endif  // SPTK_ANALYSIS_PITCH_EXTRACTION_BY_RAPT_H_
