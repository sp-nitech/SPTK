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

#ifndef SPTK_ANALYSIS_APERIODICITY_EXTRACTION_H_
#define SPTK_ANALYSIS_APERIODICITY_EXTRACTION_H_

#include <vector>  // std::vector

#include "SPTK/analysis/aperiodicity_extraction_interface.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Extract aperiodicity from waveform.
 *
 * The input is whole audio waveform and the output is the sequence of the
 * aperiodicity measure. The implemented algorithms of the extraction are
 * TANDEM-STRAIGHT and D4C.
 *
 * [1] H. Kawahara et al., &quot;Simplification and extension of non-periodic
 *     excitation source representations for high-quality speech manipulation
 *     systems,&quot; Proc. of Interspeech, pp. 38-41, 2010.
 *
 * [2] M. Morise, &quot;D4C, a band-aperiodicity estimator for high-quality
 *     speech synthesis,&quot; Proc. of Speech Communication, vol. 84,
 *     pp. 57-65, 2016.
 */
class AperiodicityExtraction {
 public:
  /**
   * Aperiodicity extraction algorithms.
   */
  enum Algorithms { kTandem = 0, kWorld, kNumAlgorithms };

  /**
   * @param[in] fft_length FFT length.
   * @param[in] frame_shift Frame shift in point.
   * @param[in] sampling_rate Sampling rate in Hz.
   * @param[in] algorithm Algorithm used for aperiodicity extraction.
   */
  AperiodicityExtraction(int fft_length, int frame_shift, double sampling_rate,
                         Algorithms algorithm);

  virtual ~AperiodicityExtraction() {
    delete aperiodicity_extraction_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return (NULL != aperiodicity_extraction_ &&
            aperiodicity_extraction_->IsValid());
  }

  /**
   * @param[in] waveform Waveform.
   * @param[in] f0 Fundamental frequency in Hz.
   * @param[out] aperiodicity Aperiodicity measure.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& waveform, const std::vector<double>& f0,
           std::vector<std::vector<double> >* aperiodicity) const;

 private:
  AperiodicityExtractionInterface* aperiodicity_extraction_;

  DISALLOW_COPY_AND_ASSIGN(AperiodicityExtraction);
};

}  // namespace sptk

#endif  // SPTK_ANALYSIS_APERIODICITY_EXTRACTION_H_
