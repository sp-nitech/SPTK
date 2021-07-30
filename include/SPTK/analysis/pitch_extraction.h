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

#ifndef SPTK_ANALYSIS_PITCH_EXTRACTION_H_
#define SPTK_ANALYSIS_PITCH_EXTRACTION_H_

#include <vector>  // std::vector

#include "SPTK/analysis/pitch_extraction_interface.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Extract pitch (fundamental frequency) from waveform.
 *
 * The input is whole audio waveform and the output is the sequence of the
 * fundamental frequency. The implemented algorithms of the extraction are
 * RAPT, SWIPE, REAPER, and DIO.
 *
 * [1] D. Talkin, &quot;A robust algorithm for pitch tracking,&quot; Speech
 *     Coding and Synthesis, pp. 497-518, 1995.
 *
 * [2] A. Camacho, &quot;SWIPE: A sawtooth waveform inspired pitch estimator
 *     for speech and music,&quot; Doctoral dissertation, 2007.
 *
 * [3] D. Talkin, &quot;REAPER: Robust epoch and pitch estimator,&quot;
 *     https://github.com/google/REAPER, 2015.
 *
 * [4] M. Morise, H. Kawahara and H. Katayose, &quot;Fast and reliable F0
 *     estimation method based on the period extraction of vocal fold vibration
 *     of singing voice and speech, Proc. of AES 35th International Conference,
 *     2009.
 */
class PitchExtraction {
 public:
  /**
   * Pitch extraction algorithm type.
   */
  enum Algorithms { kRapt = 0, kSwipe, kReaper, kWorld, kNumAlgorithms };

  /**
   * @param[in] frame_shift Frame shift in point.
   * @param[in] sampling_rate Sampling rate in Hz.
   * @param[in] lower_f0 Lower bound of F0 in Hz.
   * @param[in] upper_f0 Upper bound of F0 in Hz.
   * @param[in] voicing_threshold Threshold for determining voiced/unvoiced.
   * @param[in] algorithm Algorithm used for pitch extraction.
   */
  PitchExtraction(int frame_shift, double sampling_rate, double lower_f0,
                  double upper_f0, double voicing_threshold,
                  Algorithms algorithm);

  virtual ~PitchExtraction() {
    delete pitch_extraction_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return (NULL != pitch_extraction_ && pitch_extraction_->IsValid());
  }

  /**
   * @param[in] waveform Waveform.
   * @param[out] f0 Extracted pitch in Hz.
   * @param[out] epochs Pitchmark (valid only for REAPER).
   * @param[out] polarity Polarity (valid only for REAPER).
   */
  bool Run(const std::vector<double>& waveform, std::vector<double>* f0,
           std::vector<double>* epochs,
           PitchExtractionInterface::Polarity* polarity) const {
    return (NULL != pitch_extraction_ &&
            pitch_extraction_->Get(waveform, f0, epochs, polarity));
  }

 private:
  PitchExtractionInterface* pitch_extraction_;

  DISALLOW_COPY_AND_ASSIGN(PitchExtraction);
};

}  // namespace sptk

#endif  // SPTK_ANALYSIS_PITCH_EXTRACTION_H_
