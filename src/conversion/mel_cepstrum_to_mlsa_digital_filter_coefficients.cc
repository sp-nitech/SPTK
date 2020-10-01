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

#include "SPTK/conversion/mel_cepstrum_to_mlsa_digital_filter_coefficients.h"

#include <algorithm>  // std::copy
#include <cstddef>    // std::size_t

namespace sptk {

MelCepstrumToMlsaDigitalFilterCoefficients::
    MelCepstrumToMlsaDigitalFilterCoefficients(int num_order, double alpha)
    : num_order_(num_order), alpha_(alpha), is_valid_(true) {
  if (num_order_ < 0 || !sptk::IsValidAlpha(alpha_)) {
    is_valid_ = false;
    return;
  }
}

bool MelCepstrumToMlsaDigitalFilterCoefficients::Run(
    const std::vector<double>& mel_cepstrum,
    std::vector<double>* mlsa_digital_filter_coefficients) const {
  // Check inputs.
  const int length(num_order_ + 1);
  if (!is_valid_ || mel_cepstrum.size() != static_cast<std::size_t>(length) ||
      NULL == mlsa_digital_filter_coefficients) {
    return false;
  }

  // Prepare memories.
  if (mlsa_digital_filter_coefficients->size() !=
      static_cast<std::size_t>(length)) {
    mlsa_digital_filter_coefficients->resize(length);
  }

  // There is no need to convert input when alpha is zero.
  if (0.0 == alpha_) {
    std::copy(mel_cepstrum.begin(), mel_cepstrum.end(),
              mlsa_digital_filter_coefficients->begin());
    return true;
  }

  const double* c(&(mel_cepstrum[0]));
  double* b(&((*mlsa_digital_filter_coefficients)[0]));

  // Apply recursive formula.
  b[num_order_] = c[num_order_];
  for (int m(num_order_ - 1); 0 <= m; --m) {
    b[m] = c[m] - alpha_ * b[m + 1];
  }

  return true;
}

bool MelCepstrumToMlsaDigitalFilterCoefficients::Run(
    std::vector<double>* input_and_output) const {
  if (NULL == input_and_output) return false;
  return Run(*input_and_output, input_and_output);
}

}  // namespace sptk
