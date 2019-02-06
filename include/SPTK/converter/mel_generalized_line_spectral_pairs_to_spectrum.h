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

#ifndef SPTK_CONVERTER_MEL_GENERALIZED_LINE_SPECTRAL_PAIRS_TO_SPECTRUM_H_
#define SPTK_CONVERTER_MEL_GENERALIZED_LINE_SPECTRAL_PAIRS_TO_SPECTRUM_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

class MelGeneralizedLineSpectralPairsToSpectrum {
 public:
  //
  MelGeneralizedLineSpectralPairsToSpectrum(int num_input_order, double alpha,
                                            double gamma, int num_output_order);

  //
  virtual ~MelGeneralizedLineSpectralPairsToSpectrum() {
  }

  //
  int GetNumInputOrder() const {
    return num_input_order_;
  }

  //
  double GetAlpha() const {
    return alpha_;
  }

  //
  double GetGamma() const {
    return gamma_;
  }

  //
  int GetNumOutputOrder() const {
    return num_output_order_;
  }

  //
  bool IsValid() const {
    return is_valid_;
  }

  // Assume that the first element of line_spectral_pairs is linear gain and
  // the other elements are in normalized frequency (0...pi).
  bool Run(const std::vector<double>& line_spectral_pairs,
           std::vector<double>* spectrum) const;

 private:
  //
  const int num_input_order_;

  //
  const double alpha_;

  //
  const double gamma_;

  //
  const int num_output_order_;

  //
  bool is_valid_;

  //
  DISALLOW_COPY_AND_ASSIGN(MelGeneralizedLineSpectralPairsToSpectrum);
};

}  // namespace sptk

#endif  // SPTK_CONVERTER_MEL_GENERALIZED_LINE_SPECTRAL_PAIRS_TO_SPECTRUM_H_
