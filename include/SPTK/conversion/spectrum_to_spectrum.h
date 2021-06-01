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

#ifndef SPTK_CONVERSION_SPECTRUM_TO_SPECTRUM_H_
#define SPTK_CONVERSION_SPECTRUM_TO_SPECTRUM_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

class SpectrumToSpectrum {
 public:
  //
  enum InputOutputFormats {
    kLogAmplitudeSpectrumInDecibels = 0,
    kLogAmplitudeSpectrum,
    kAmplitudeSpectrum,
    kPowerSpectrum,
    kNumInputOutputFormats
  };

  class OperationInterface {
   public:
    virtual ~OperationInterface() {
    }
    virtual bool Run(std::vector<double>* input_and_output) const = 0;
  };

  //
  SpectrumToSpectrum(int fft_length, InputOutputFormats input_format,
                     InputOutputFormats output_format,
                     double epsilon_for_calculating_logarithms,
                     double relative_floor_in_decibels);

  //
  virtual ~SpectrumToSpectrum() {
    for (std::vector<SpectrumToSpectrum::OperationInterface*>::iterator itr(
             operations_.begin());
         itr != operations_.end(); ++itr) {
      delete (*itr);
    }
  }

  //
  int GetFftLength() const {
    return fft_length_;
  }

  //
  InputOutputFormats GetInputFormat() const {
    return input_format_;
  }

  //
  InputOutputFormats GetOutputFormat() const {
    return output_format_;
  }

  //
  double GetEpsilonForCalculatingLogarithms() const {
    return epsilon_for_calculating_logarithms_;
  }

  //
  double GetRelativeFloorInDecibels() const {
    return relative_floor_in_decibels_;
  }

  //
  bool IsValid() const {
    return is_valid_;
  }

  //
  bool Run(const std::vector<double>& input, std::vector<double>* output) const;

  //
  bool Run(std::vector<double>* input_and_output) const;

 private:
  //
  const int fft_length_;

  //
  const InputOutputFormats input_format_;

  //
  const InputOutputFormats output_format_;

  //
  const double epsilon_for_calculating_logarithms_;

  //
  const double relative_floor_in_decibels_;

  //
  std::vector<SpectrumToSpectrum::OperationInterface*> operations_;

  //
  bool is_valid_;

  //
  DISALLOW_COPY_AND_ASSIGN(SpectrumToSpectrum);
};

}  // namespace sptk

#endif  // SPTK_CONVERSION_SPECTRUM_TO_SPECTRUM_H_
