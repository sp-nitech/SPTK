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

#include "SPTK/conversion/mel_generalized_line_spectral_pairs_to_spectrum.h"

#include <cmath>    // std::atan, std::cos, std::fabs, std::sin
#include <cstddef>  // std::size_t

namespace {

double Warp(double omega, double alpha) {
  if (0.0 == alpha) return omega;

  return omega +
         2.0 * std::atan(alpha * std::sin(omega) /
                         (1.0 - alpha * std::cos(omega)));
}

}  // namespace

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

    const double warped_omega(Warp(omega, alpha_));
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
