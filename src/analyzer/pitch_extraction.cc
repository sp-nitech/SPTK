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
//                1996-2018  Nagoya Institute of Technology          //
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

#include "SPTK/analyzer/pitch_extraction.h"

#include "SPTK/analyzer/pitch_extraction_by_rapt.h"
#include "SPTK/analyzer/pitch_extraction_by_reaper.h"
#include "SPTK/analyzer/pitch_extraction_by_swipe.h"
#include "SPTK/analyzer/pitch_extraction_by_world.h"

namespace sptk {

PitchExtraction::PitchExtraction(int frame_shift, double sampling_rate,
                                 double minimum_f0, double maximum_f0,
                                 double voicing_threshold,
                                 PitchExtraction::Algorithms algorithm) {
  switch (algorithm) {
    case kRapt: {
      pitch_extractor_ =
          new PitchExtractionByRapt(frame_shift, sampling_rate, minimum_f0,
                                    maximum_f0, voicing_threshold);
      break;
    }
    case kSwipe: {
      pitch_extractor_ =
          new PitchExtractionBySwipe(frame_shift, sampling_rate, minimum_f0,
                                     maximum_f0, voicing_threshold);
      break;
    }
    case kReaper: {
      pitch_extractor_ =
          new PitchExtractionByReaper(frame_shift, sampling_rate, minimum_f0,
                                      maximum_f0, voicing_threshold);
      break;
    }
    case kWorld: {
      pitch_extractor_ =
          new PitchExtractionByWorld(frame_shift, sampling_rate, minimum_f0,
                                     maximum_f0, voicing_threshold);
      break;
    }
    default: {
      pitch_extractor_ = NULL;
      break;
    }
  }
}

}  // namespace sptk
