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

#include "SPTK/conversion/generalized_cepstrum_inverse_gain_normalization.h"

#include <algorithm>  // std::copy, std::transform
#include <cmath>      // std::log, std::pow
#include <cstddef>    // std::size_t

namespace sptk {

GeneralizedCepstrumInverseGainNormalization::
    GeneralizedCepstrumInverseGainNormalization(int num_order, double gamma)
    : num_order_(num_order), gamma_(gamma), is_valid_(true) {
  if (num_order_ < 0 || !sptk::IsValidGamma(gamma_)) {
    is_valid_ = false;
    return;
  }
}

bool GeneralizedCepstrumInverseGainNormalization::Run(
    const std::vector<double>& normalized_generalized_cepstrum,
    std::vector<double>* generalized_cepstrum) const {
  // Check inputs.
  if (!is_valid_ ||
      normalized_generalized_cepstrum.size() !=
          static_cast<std::size_t>(num_order_ + 1) ||
      NULL == generalized_cepstrum) {
    return false;
  }

  // Prepare memories.
  if (generalized_cepstrum->size() !=
      static_cast<std::size_t>(num_order_ + 1)) {
    generalized_cepstrum->resize(num_order_ + 1);
  }

  if (0.0 == gamma_) {
    (*generalized_cepstrum)[0] = std::log(normalized_generalized_cepstrum[0]);
    std::copy(normalized_generalized_cepstrum.begin() + 1,
              normalized_generalized_cepstrum.end(),
              generalized_cepstrum->begin() + 1);
  } else {
    const double z(std::pow(normalized_generalized_cepstrum[0], gamma_));
    (*generalized_cepstrum)[0] = (z - 1.0) / gamma_;
    std::transform(normalized_generalized_cepstrum.begin() + 1,
                   normalized_generalized_cepstrum.end(),
                   generalized_cepstrum->begin() + 1,
                   [z](double c) { return c * z; });
  }

  return true;
}

bool GeneralizedCepstrumInverseGainNormalization::Run(
    std::vector<double>* input_and_output) const {
  if (NULL == input_and_output) return false;
  return Run(*input_and_output, input_and_output);
}

}  // namespace sptk
