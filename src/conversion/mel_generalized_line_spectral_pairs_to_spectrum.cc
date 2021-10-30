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

#include "SPTK/conversion/mel_generalized_line_spectral_pairs_to_spectrum.h"

#include <cmath>    // std::cos, std::fabs, std::sin
#include <cstddef>  // std::size_t

namespace sptk {

MelGeneralizedLineSpectralPairsToSpectrum::
    MelGeneralizedLineSpectralPairsToSpectrum(int num_order, double alpha,
                                              double gamma, int fft_length)
    : num_order_(num_order),
      alpha_(alpha),
      gamma_(gamma),
      fft_length_(fft_length),
      is_valid_(true) {
  if (num_order_ < 0 || !sptk::IsValidAlpha(alpha_) ||
      !sptk::IsValidGamma(gamma_) || fft_length_ <= 0) {
    is_valid_ = false;
    return;
  }
}

bool MelGeneralizedLineSpectralPairsToSpectrum::Run(
    const std::vector<double>& mel_generalized_line_spectral_pairs,
    std::vector<double>* spectrum) const {
  // Check inputs.
  if (!is_valid_ ||
      mel_generalized_line_spectral_pairs.size() !=
          static_cast<std::size_t>(num_order_ + 1) ||
      NULL == spectrum) {
    return false;
  }

  // Prepare memories.
  const int output_length(fft_length_ / 2 + 1);
  if (spectrum->size() != static_cast<std::size_t>(output_length)) {
    spectrum->resize(output_length);
  }

  const double* w(&(mel_generalized_line_spectral_pairs[0]));
  double* output(&((*spectrum)[0]));

  const bool is_odd(!sptk::IsEven(num_order_));
  const double c0(sptk::FloorLog(w[0]));
  const double c1(0.5 / gamma_);
  const double c2(is_odd ? (num_order_ - 1) * sptk::kLogTwo
                         : num_order_ * sptk::kLogTwo);
  const double delta(sptk::kPi / (output_length - 1));
  double omega(0.0);

  for (int j(0); j < output_length; ++j, omega += delta) {
    double p(0.0);
    double q(0.0);

    const double warped_omega(sptk::Warp(omega, alpha_));
    if (is_odd) {
      p += 2.0 * sptk::FloorLog(std::sin(warped_omega));
    } else {
      p += 2.0 * sptk::FloorLog(std::sin(warped_omega * 0.5));
      q += 2.0 * sptk::FloorLog(std::cos(warped_omega * 0.5));
    }

    const double cos_omega(std::cos(warped_omega));
    for (int i(2); i <= num_order_; i += 2) {
      p += 2.0 * sptk::FloorLog(std::fabs(cos_omega - std::cos(w[i])));
    }
    for (int i(1); i <= num_order_; i += 2) {
      q += 2.0 * sptk::FloorLog(std::fabs(cos_omega - std::cos(w[i])));
    }

    output[j] = c0 + c1 * (c2 + sptk::AddInLogSpace(p, q));
  }

  return true;
}

}  // namespace sptk
