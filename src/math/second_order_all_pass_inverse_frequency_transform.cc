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

#include "SPTK/math/second_order_all_pass_inverse_frequency_transform.h"

#include <algorithm>  // std::copy, std::fill
#include <cmath>      // std::cos, std::sin
#include <cstddef>    // std::size_t
#include <numeric>    // std::inner_product

#include "SPTK/math/inverse_fast_fourier_transform.h"

namespace sptk {

SecondOrderAllPassInverseFrequencyTransform::
    SecondOrderAllPassInverseFrequencyTransform(int num_input_order,
                                                int num_output_order,
                                                int fft_length, double alpha,
                                                double theta)
    : num_input_order_(num_input_order),
      num_output_order_(num_output_order),
      alpha_(alpha),
      theta_(theta),
      is_valid_(true) {
  if (num_input_order_ < 0 || num_output_order_ < 0 ||
      fft_length <= num_output_order_ || !sptk::IsValidAlpha(alpha_) ||
      !sptk::IsInRange(theta_, 0.0, sptk::kPi)) {
    is_valid_ = false;
    return;
  }

  const int input_length(num_input_order_ + 1);
  const int output_length(num_output_order_ + 1);
  std::vector<std::vector<double> > real(2 * num_input_order_ + 1,
                                         std::vector<double>(fft_length));
  std::vector<std::vector<double> > imag(2 * num_input_order_ + 1,
                                         std::vector<double>(fft_length));
  {
    std::vector<double> ww(fft_length);
    {
      const double delta(sptk::kTwoPi / fft_length);
      for (int j(0); j < fft_length; ++j) {
        ww[j] = sptk::Warp(delta * j, alpha, theta);
      }
    }

    sptk::InverseFastFourierTransform inverse_fourier_transform(fft_length);
    for (int m1(-num_input_order_); m1 <= num_input_order_; ++m1) {
      const int m(m1 + num_input_order_);
      for (int j(0); j < fft_length; ++j) {
        real[m][j] = std::cos(ww[j] * m1);
        imag[m][j] = -std::sin(ww[j] * m1);
      }

      if (!inverse_fourier_transform.Run(&real[m], &imag[m])) {
        is_valid_ = false;
        return;
      }

      if (0 < m1) {
        for (int j(0); j <= num_output_order_; ++j) {
          real[m][j] += real[num_input_order_ - m1][j];
        }
      }
    }
  }

  conversion_matrix_.resize(output_length);
  for (int m2(0); m2 <= num_output_order_; ++m2) {
    conversion_matrix_[m2].resize(input_length);
    for (int m1(0); m1 <= num_input_order_; ++m1) {
      conversion_matrix_[m2][m1] = real[m1 + num_input_order_][m2];
    }
  }
  for (int m1(1); m1 <= num_input_order_; ++m1) {
    conversion_matrix_[0][m1] *= 0.5;
  }
  for (int m2(1); m2 <= num_output_order_; ++m2) {
    conversion_matrix_[m2][0] *= 2.0;
  }
}

bool SecondOrderAllPassInverseFrequencyTransform::Run(
    const std::vector<double>& warped_sequence,
    std::vector<double>* minimum_phase_sequence) const {
  // Check inputs.
  const int input_length(num_input_order_ + 1);
  if (!is_valid_ ||
      warped_sequence.size() != static_cast<std::size_t>(input_length) ||
      NULL == minimum_phase_sequence) {
    return false;
  }

  // Prepare memories.
  const int output_length(num_output_order_ + 1);
  if (minimum_phase_sequence->size() !=
      static_cast<std::size_t>(output_length)) {
    minimum_phase_sequence->resize(output_length);
  }

  // There is no need to convert input when alpha and theta are zeros.
  if (0.0 == alpha_ && 0.0 == theta_) {
    if (num_input_order_ < num_output_order_) {
      std::copy(warped_sequence.begin(), warped_sequence.end(),
                minimum_phase_sequence->begin());
      std::fill(minimum_phase_sequence->begin() + input_length,
                minimum_phase_sequence->end(), 0.0);
    } else {
      std::copy(warped_sequence.begin(),
                warped_sequence.begin() + output_length,
                minimum_phase_sequence->begin());
    }
    return true;
  }

  // Perform inverse frequency transform.
  for (int m2(0); m2 <= num_output_order_; ++m2) {
    (*minimum_phase_sequence)[m2] = std::inner_product(
        conversion_matrix_[m2].begin(), conversion_matrix_[m2].end(),
        warped_sequence.begin(), 0.0);
  }

  return true;
}

}  // namespace sptk
