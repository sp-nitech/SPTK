// ------------------------------------------------------------------------ //
// Copyright 2021 SPTK Working Group                                        //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ------------------------------------------------------------------------ //

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
