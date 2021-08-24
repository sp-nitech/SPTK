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

#ifndef SPTK_ANALYSIS_PITCH_EXTRACTION_INTERFACE_H_
#define SPTK_ANALYSIS_PITCH_EXTRACTION_INTERFACE_H_

#include <vector>  // std::vector

namespace sptk {

/**
 * An interface of pitch extraction.
 */
class PitchExtractionInterface {
 public:
  /**
   * Polarity.
   */
  enum Polarity {
    kUnknown = 0,
    kPositive,
    kNegative,
  };

  virtual ~PitchExtractionInterface() {
  }

  /**
   * @return True if this object is valid.
   */
  virtual bool IsValid() const = 0;

  /**
   * @param[in] waveform Waveform samples.
   * @param[out] f0 F0.
   * @param[out] epochs Epochs.
   * @param[out] polarity Polarity.
   * @return True on success, false on failure.
   */
  virtual bool Get(const std::vector<double>& waveform, std::vector<double>* f0,
                   std::vector<double>* epochs, Polarity* polarity) const = 0;
};

}  // namespace sptk

#endif  // SPTK_ANALYSIS_PITCH_EXTRACTION_INTERFACE_H_
