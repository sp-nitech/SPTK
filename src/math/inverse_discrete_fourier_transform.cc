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

#include "SPTK/math/inverse_discrete_fourier_transform.h"

#include <cmath>    // std::cos, std::sin
#include <cstddef>  // std::size_t
#include <vector>   // std::vector

namespace sptk {

InverseDiscreteFourierTransform::InverseDiscreteFourierTransform(int dft_length)
    : dft_length_(dft_length), is_valid_(true) {
  if (dft_length_ <= 0) {
    is_valid_ = false;
    return;
  }

  sine_table_.resize(dft_length_);
  cosine_table_.resize(dft_length_);
  for (int i(0); i < dft_length_; ++i) {
    const double argument(sptk::kTwoPi * i / dft_length_);
    sine_table_[i] = std::sin(argument);
    cosine_table_[i] = std::cos(argument);
  }
}

bool InverseDiscreteFourierTransform::Run(
    const std::vector<double>& real_part_input,
    const std::vector<double>& imag_part_input,
    std::vector<double>* real_part_output,
    std::vector<double>* imag_part_output) const {
  // Check inputs
  if (!is_valid_ ||
      real_part_input.size() != static_cast<std::size_t>(dft_length_) ||
      imag_part_input.size() != static_cast<std::size_t>(dft_length_) ||
      NULL == real_part_output || NULL == imag_part_output) {
    return false;
  }

  // Prepare memories.
  if (real_part_output->size() != static_cast<std::size_t>(dft_length_)) {
    real_part_output->resize(dft_length_);
  }
  if (imag_part_output->size() != static_cast<std::size_t>(dft_length_)) {
    imag_part_output->resize(dft_length_);
  }

  const double* input_x(&(real_part_input[0]));
  const double* input_y(&(imag_part_input[0]));
  double* output_x(&((*real_part_output)[0]));
  double* output_y(&((*imag_part_output)[0]));

  for (int n(0); n < dft_length_; ++n) {
    double sum_x(0.0);
    double sum_y(0.0);
    for (int k(0); k < dft_length_; ++k) {
      const int index(k * n % dft_length_);
      sum_x +=
          input_x[k] * cosine_table_[index] + input_y[k] * sine_table_[index];
      sum_y +=
          input_x[k] * sine_table_[index] - input_y[k] * cosine_table_[index];
    }
    output_x[n] = sum_x / dft_length_;
    output_y[n] = sum_y / dft_length_;
  }

  return true;
}

bool InverseDiscreteFourierTransform::Run(
    std::vector<double>* real_part, std::vector<double>* imag_part) const {
  if (NULL == real_part || NULL == imag_part) return false;
  std::vector<double> real_part_input(*real_part);
  std::vector<double> imag_part_input(*imag_part);
  return Run(real_part_input, imag_part_input, real_part, imag_part);
}

}  // namespace sptk
