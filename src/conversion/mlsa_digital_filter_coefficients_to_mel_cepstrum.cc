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
