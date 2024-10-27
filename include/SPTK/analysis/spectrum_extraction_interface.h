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

#ifndef SPTK_ANALYSIS_SPECTRUM_EXTRACTION_INTERFACE_H_
#define SPTK_ANALYSIS_SPECTRUM_EXTRACTION_INTERFACE_H_

#include <vector>  // std::vector

namespace sptk {

/**
 * An interface of spectrum extraction.
 */
class SpectrumExtractionInterface {
 public:
  virtual ~SpectrumExtractionInterface() = default;

  /**
   * @return Frame shift in point.
   */
  virtual int GetFrameShift() const = 0;

  /**
   * @return True if this object is valid.
   */
  virtual bool IsValid() const = 0;

  /**
   * @param[in] waveform Waveform.
   * @param[in] f0 Fundamental frequency in Hz.
   * @param[out] spectrum Power spectrum.
   * @return True on success, false on failure.
   */
  virtual bool Run(const std::vector<double>& waveform,
                   const std::vector<double>& f0,
                   std::vector<std::vector<double> >* spectrum) const = 0;
};

}  // namespace sptk

#endif  // SPTK_ANALYSIS_SPECTRUM_EXTRACTION_INTERFACE_H_
