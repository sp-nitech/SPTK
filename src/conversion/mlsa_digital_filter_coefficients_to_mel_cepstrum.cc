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

#include "SPTK/conversion/mlsa_digital_filter_coefficients_to_mel_cepstrum.h"

#include <algorithm>  // std::copy
#include <cstddef>    // std::size_t

namespace sptk {

MlsaDigitalFilterCoefficientsToMelCepstrum::
    MlsaDigitalFilterCoefficientsToMelCepstrum(int num_order, double alpha)
    : num_order_(num_order), alpha_(alpha), is_valid_(true) {
  if (num_order_ < 0 || !sptk::IsValidAlpha(alpha_)) {
    is_valid_ = false;
    return;
  }
}

bool MlsaDigitalFilterCoefficientsToMelCepstrum::Run(
    const std::vector<double>& mlsa_digital_filter_coefficients,
    std::vector<double>* mel_cepstrum) const {
  // Check inputs.
  const int length(num_order_ + 1);
  if (!is_valid_ ||
      mlsa_digital_filter_coefficients.size() !=
          static_cast<std::size_t>(length) ||
      NULL == mel_cepstrum) {
    return false;
  }

  // Prepare memories.
  if (mel_cepstrum->size() != static_cast<std::size_t>(length)) {
    mel_cepstrum->resize(length);
  }

  // There is no need to convert input when alpha is zero.
  if (0.0 == alpha_) {
    std::copy(mlsa_digital_filter_coefficients.begin(),
              mlsa_digital_filter_coefficients.end(), mel_cepstrum->begin());
    return true;
  }

  const double* b(&(mlsa_digital_filter_coefficients[0]));
  double* c(&((*mel_cepstrum)[0]));

  c[num_order_] = b[num_order_];
  for (int m(num_order_ - 1); 0 <= m; --m) {
    c[m] = b[m] + alpha_ * b[m + 1];
  }

  return true;
}

bool MlsaDigitalFilterCoefficientsToMelCepstrum::Run(
    std::vector<double>* input_and_output) const {
  if (NULL == input_and_output) return false;
  std::vector<double> input(*input_and_output);
  return Run(input, input_and_output);
}

}  // namespace sptk
