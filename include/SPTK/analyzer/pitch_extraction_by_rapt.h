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
//                1996-2020  Nagoya Institute of Technology          //
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

#ifndef SPTK_ANALYZER_PITCH_EXTRACTION_BY_RAPT_H_
#define SPTK_ANALYZER_PITCH_EXTRACTION_BY_RAPT_H_

#include <vector>  // std::vector

#include "SPTK/analyzer/pitch_extraction_interface.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

class PitchExtractionByRapt : public PitchExtractionInterface {
 public:
  /**
   * @param[in] frame_shift Frame shift in point.
   * @param[in] sampling_rate Sampling rate in Hz.
   * @param[in] lower_f0 Lower bound of F0 in Hz.
   * @param[in] upper_f0 Upper bound of F0 in Hz.
   * @param[in] voicing_threshold Threshold for determining voiced/unvoiced.
   * @param[in] algorithm Algorithm used for pitch extraction.
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
   * @return True if this obejct is valid.
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

#endif  // SPTK_ANALYZER_PITCH_EXTRACTION_BY_RAPT_H_
