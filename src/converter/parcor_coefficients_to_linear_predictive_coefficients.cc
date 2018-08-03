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

#include "SPTK/converter/parcor_coefficients_to_linear_predictive_coefficients.h"

#include <algorithm>  // std::copy
#include <cstddef>    // std::size_t

namespace sptk {

bool ParcorCoefficientsToLinearPredictiveCoefficients::Run(
    const std::vector<double>& parcor_coefficients,
    std::vector<double>* linear_predictive_coefficients,
    ParcorCoefficientsToLinearPredictiveCoefficients::Buffer* buffer) const {
  // check inputs
  if (!is_valid_ ||
      parcor_coefficients.size() != static_cast<std::size_t>(num_order_ + 1) ||
      NULL == linear_predictive_coefficients || NULL == buffer) {
    return false;
  }

  // prepare memory
  const int output_length(num_order_ + 1);
  if (linear_predictive_coefficients->size() !=
      static_cast<std::size_t>(output_length)) {
    linear_predictive_coefficients->resize(output_length);
  }

  (*linear_predictive_coefficients)[0] = parcor_coefficients[0];
  if (0 == num_order_) {
    return true;
  }

  // prepare buffer
  if (buffer->k_.size() != static_cast<std::size_t>(output_length)) {
    buffer->k_.resize(output_length);
  }

  // get value
  double* output(&((*linear_predictive_coefficients)[0]));

  // transform parcor coefficients to linear predictive coefficients
  std::copy(parcor_coefficients.begin(), parcor_coefficients.end(),
            buffer->k_.begin());

  double* k(&buffer->k_[0]);
  for (int i(1); i <= num_order_; ++i) {
    for (int j(1); j < i; ++j) {
      output[j] = k[j] + k[i] * k[i - j];
    }
    for (int j(1); j < i; ++j) {
      k[j] = output[j];
    }
  }
  output[num_order_] = k[num_order_];

  return true;
}

}  // namespace sptk
