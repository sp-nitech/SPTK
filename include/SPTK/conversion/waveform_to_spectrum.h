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
//                1996-2019  Nagoya Institute of Technology          //
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

#ifndef SPTK_CONVERSION_WAVEFORM_TO_SPECTRUM_H_
#define SPTK_CONVERSION_WAVEFORM_TO_SPECTRUM_H_

#include <vector>  // std::vector

#include "SPTK/conversion/filter_coefficients_to_spectrum.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

class WaveformToSpectrum {
 public:
  class Buffer {
   public:
    Buffer() {
    }
    virtual ~Buffer() {
    }

   private:
    FilterCoefficientsToSpectrum::Buffer buffer_;
    friend class WaveformToSpectrum;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  //
  WaveformToSpectrum(int frame_length, int fft_length,
                     FilterCoefficientsToSpectrum::OutputFormats output_format,
                     double epsilon_for_calculating_logarithms,
                     double relative_floor_in_decibels);

  //
  virtual ~WaveformToSpectrum() {
  }

  //
  int GetFrameLength() const {
    return filter_coefficients_to_spectrum_.GetNumNumeratorOrder() + 1;
  }

  //
  int GetFftLength() const {
    return filter_coefficients_to_spectrum_.GetFftLength();
  }

  //
  FilterCoefficientsToSpectrum::OutputFormats GetOutputFormat() const {
    return filter_coefficients_to_spectrum_.GetOutputFormat();
  }

  //
  double GetEpsilonForCalculatingLogarithms() const {
    return filter_coefficients_to_spectrum_
        .GetEpsilonForCalculatingLogarithms();
  }

  //
  double GetRelativeFloorInDecibels() const {
    return filter_coefficients_to_spectrum_.GetRelativeFloorInDecibels();
  }

  //
  bool IsValid() const {
    return filter_coefficients_to_spectrum_.IsValid();
  }

  //
  bool Run(const std::vector<double>& waveform, std::vector<double>* spectrum,
           WaveformToSpectrum::Buffer* buffer) const;

 private:
  //
  const FilterCoefficientsToSpectrum filter_coefficients_to_spectrum_;

  //
  const std::vector<double> dummy_for_filter_coefficients_to_spectrum_;

  //
  DISALLOW_COPY_AND_ASSIGN(WaveformToSpectrum);
};

}  // namespace sptk

#endif  // SPTK_CONVERSION_WAVEFORM_TO_SPECTRUM_H_
