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

#include "SPTK/math/real_valued_inverse_fast_fourier_transform.h"

#include <algorithm>  // std::transform

namespace sptk {

RealValuedInverseFastFourierTransform::RealValuedInverseFastFourierTransform(
    int fft_length)
    : RealValuedInverseFastFourierTransform(fft_length - 1, fft_length) {
}

RealValuedInverseFastFourierTransform::RealValuedInverseFastFourierTransform(
    int num_order, int fft_length)
    : fast_fourier_transform_(num_order, fft_length) {
}

bool RealValuedInverseFastFourierTransform::Run(
    const std::vector<double>& real_part_input,
    std::vector<double>* real_part_output,
    std::vector<double>* imag_part_output,
    RealValuedInverseFastFourierTransform::Buffer* buffer) const {
  if (NULL == buffer) {
    return false;
  }

  if (!fast_fourier_transform_.Run(real_part_input, real_part_output,
                                   imag_part_output,
                                   &buffer->fast_fourier_transform_buffer_)) {
    return false;
  }

  const int fft_length(fast_fourier_transform_.GetFftLength());
  const double z(1.0 / fft_length);
  std::transform(real_part_output->begin(),
                 real_part_output->begin() + fft_length,
                 real_part_output->begin(), [z](double x) { return x * z; });
  std::transform(imag_part_output->begin(),
                 imag_part_output->begin() + fft_length,
                 imag_part_output->begin(), [z](double x) { return x * z; });

  return true;
}

bool RealValuedInverseFastFourierTransform::Run(
    std::vector<double>* real_part, std::vector<double>* imag_part,
    RealValuedInverseFastFourierTransform::Buffer* buffer) const {
  if (NULL == real_part) return false;
  return Run(*real_part, real_part, imag_part, buffer);
}

}  // namespace sptk
