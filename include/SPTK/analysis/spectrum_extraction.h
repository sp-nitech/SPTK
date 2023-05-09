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

#ifndef SPTK_ANALYSIS_SPECTRUM_EXTRACTION_H_
#define SPTK_ANALYSIS_SPECTRUM_EXTRACTION_H_

#include <vector>  // std::vector

#include "SPTK/analysis/spectrum_extraction_interface.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Extract spectrum from waveform.
 *
 * The input is whole audio waveform and the output is the sequence of the
 * spectrum measure. The implemented algorithm of the extraction is CheapTrick.
 *
 * [1] M. Morise, &quot;CheapTrick, a spectral envelope estimator for
 *     high-quality speech synthesis,&quot; Proc. of Speech Communication,
 *     vol. 67, pp. 1-7, 2015.
 */
class SpectrumExtraction {
 public:
  /**
   * Spectrum extraction algorithm type.
   */
  enum Algorithms { kWorld = 0, kNumAlgorithms };

  /**
   * @param[in] fft_length FFT length.
   * @param[in] frame_shift Frame shift in point.
   * @param[in] sampling_rate Sampling rate in Hz.
   * @param[in] algorithm Algorithm used for spectrum extraction.
   */
  SpectrumExtraction(int fft_length, int frame_shift, double sampling_rate,
                     Algorithms algorithm);

  virtual ~SpectrumExtraction() {
    delete spectrum_extraction_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return (NULL != spectrum_extraction_ && spectrum_extraction_->IsValid());
  }

  /**
   * @param[in] waveform Waveform.
   * @param[in] f0 Fundamental frequency in Hz.
   * @param[out] spectrum Power spectrum.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& waveform, const std::vector<double>& f0,
           std::vector<std::vector<double> >* spectrum) const;

 private:
  SpectrumExtractionInterface* spectrum_extraction_;

  DISALLOW_COPY_AND_ASSIGN(SpectrumExtraction);
};

}  // namespace sptk

#endif  // SPTK_ANALYSIS_SPECTRUM_EXTRACTION_H_
