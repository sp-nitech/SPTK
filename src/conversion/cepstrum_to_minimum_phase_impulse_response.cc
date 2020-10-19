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

#include "SPTK/conversion/cepstrum_to_minimum_phase_impulse_response.h"

#include <cmath>    // std::exp
#include <cstddef>  // std::size_t

namespace sptk {

CepstrumToMinimumPhaseImpulseResponse::CepstrumToMinimumPhaseImpulseResponse(
    int num_input_order, int num_output_order)
    : num_input_order_(num_input_order),
      num_output_order_(num_output_order),
      is_valid_(true) {
  if (num_input_order_ < 0 || num_output_order_ < 0) {
    is_valid_ = false;
    return;
  }
}

bool CepstrumToMinimumPhaseImpulseResponse::Run(
    const std::vector<double>& cepstrum,
    std::vector<double>* minimum_phase_impulse_response) const {
  // Check inputs.
  if (!is_valid_ ||
      cepstrum.size() != static_cast<std::size_t>(num_input_order_ + 1) ||
      NULL == minimum_phase_impulse_response) {
    return false;
  }

  // Prepare memories.
  if (minimum_phase_impulse_response->size() !=
      static_cast<std::size_t>(num_output_order_ + 1)) {
    minimum_phase_impulse_response->resize(num_output_order_ + 1);
  }

  const double* c(&cepstrum[0]);
  double* h(&((*minimum_phase_impulse_response)[0]));

  h[0] = std::exp(c[0]);
  for (int n(1); n <= num_output_order_; ++n) {
    double sum(0.0);
    const int end((num_input_order_ < n) ? num_input_order_ : n);
    for (int k(1); k <= end; ++k) {
      sum += k * c[k] * h[n - k];
    }
    h[n] = sum / n;
  }

  return true;
}

}  // namespace sptk
