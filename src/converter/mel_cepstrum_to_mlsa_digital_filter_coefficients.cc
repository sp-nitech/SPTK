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

#include "SPTK/converter/mel_cepstrum_to_mlsa_digital_filter_coefficients.h"

namespace sptk {

MelCepstrumToMlsaDigitalFilterCoefficients::
    MelCepstrumToMlsaDigitalFilterCoefficients(int num_order, double alpha)
    : num_order_(num_order), alpha_(alpha), is_valid_(true) {
  if (num_order_ < 0) {
    is_valid_ = false;
  }
}

bool MelCepstrumToMlsaDigitalFilterCoefficients::Run(
    const std::vector<double>& mel_cepstrum,
    std::vector<double>* mlsa_digital_filter_coefficients) const {
  // check inputs
  if (!is_valid_ ||
      mel_cepstrum.size() != static_cast<std::size_t>(num_order_ + 1) ||
      NULL == mlsa_digital_filter_coefficients) {
    return false;
  }

  // prepare memory
  if (mlsa_digital_filter_coefficients->size() <
      static_cast<std::size_t>(num_order_ + 1)) {
    mlsa_digital_filter_coefficients->resize(num_order_ + 1);
  }

  // get values
  const double* input(&(mel_cepstrum[0]));
  double* output(&((*mlsa_digital_filter_coefficients)[0]));

  output[num_order_] = input[num_order_];

  for (int i(num_order_ - 1); 0 <= i; --i) {
    output[i] = input[i] - alpha_ * output[i + 1];
  }

  return true;
}

}  // namespace sptk
