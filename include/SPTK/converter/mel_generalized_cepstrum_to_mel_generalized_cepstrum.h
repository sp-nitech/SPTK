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

#ifndef SPTK_CONVERTER_MEL_GENERALIZED_CEPSTRUM_TO_MEL_GENERALIZED_CEPSTRUM_H_
#define SPTK_CONVERTER_MEL_GENERALIZED_CEPSTRUM_TO_MEL_GENERALIZED_CEPSTRUM_H_

#include <vector>  // std::vector

#include "SPTK/math/frequency_transform.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

class MelGeneralizedCepstrumToMelGeneralizedCepstrum {
 public:
  class Buffer {
   public:
    Buffer() {
    }
    virtual ~Buffer() {
    }

   private:
    FrequencyTransform::Buffer frequency_transform_buffer_;
    std::vector<double> temporary_mel_generalized_cepstrum_;
    friend class MelGeneralizedCepstrumToMelGeneralizedCepstrum;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  class ModuleInterface {
   public:
    virtual ~ModuleInterface() {
    }
    virtual bool IsValid() const = 0;
    virtual bool Run(
        const std::vector<double>& input, std::vector<double>* output,
        FrequencyTransform::Buffer* frequency_transform_buffer) const = 0;
  };

  //
  MelGeneralizedCepstrumToMelGeneralizedCepstrum(
      int num_input_order_, double input_alpha_, double input_gamma_,
      bool is_normalized_input_, bool is_multiplied_input_,
      int num_output_order_, double output_alpha_, double output_gamma_,
      bool is_normalized_output_, bool is_multiplied_output_);

  //
  virtual ~MelGeneralizedCepstrumToMelGeneralizedCepstrum() {
    for (std::vector<MelGeneralizedCepstrumToMelGeneralizedCepstrum::
                         ModuleInterface*>::iterator itr(modules_.begin());
         itr != modules_.end(); ++itr) {
      delete (*itr);
    }
  }

  //
  int GetNumInputOrder() const {
    return num_input_order_;
  }

  //
  double GetInputAlpha() const {
    return input_alpha_;
  }

  //
  double GetInputGamma() const {
    return input_gamma_;
  }

  //
  bool IsNormalizedInput() const {
    return is_normalized_input_;
  }

  //
  bool IsMultipliedInput() const {
    return is_multiplied_input_;
  }

  //
  int GetNumOutputOrder() const {
    return num_output_order_;
  }

  //
  double GetOutputAlpha() const {
    return output_alpha_;
  }

  //
  double GetOutputGamma() const {
    return output_gamma_;
  }

  //
  bool IsNormalizedOutut() const {
    return is_normalized_output_;
  }

  //
  bool IsMultipliedOutput() const {
    return is_multiplied_output_;
  }

  //
  bool IsValid() const {
    return is_valid_;
  }

  //
  bool Run(
      const std::vector<double>& input, std::vector<double>* output,
      MelGeneralizedCepstrumToMelGeneralizedCepstrum::Buffer* buffer) const;

 private:
  //
  const int num_input_order_;

  //
  const double input_alpha_;

  //
  const double input_gamma_;

  //
  const bool is_normalized_input_;

  //
  const bool is_multiplied_input_;

  //
  const int num_output_order_;

  //
  const double output_alpha_;

  //
  const double output_gamma_;

  //
  const bool is_normalized_output_;

  //
  const bool is_multiplied_output_;

  //
  double alpha_transform_;

  //
  std::vector<MelGeneralizedCepstrumToMelGeneralizedCepstrum::ModuleInterface*>
      modules_;

  //
  bool is_valid_;

  //
  DISALLOW_COPY_AND_ASSIGN(MelGeneralizedCepstrumToMelGeneralizedCepstrum);
};

}  // namespace sptk

#endif  // SPTK_CONVERTER_MEL_GENERALIZED_CEPSTRUM_TO_MEL_GENERALIZED_CEPSTRUM_H_
