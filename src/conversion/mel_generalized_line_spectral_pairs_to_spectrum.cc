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
//                1996-2019  Nagoya Institute of Technology          //
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

namespace sptk {

MelGeneralizedLineSpectralPairsToSpectrum::
    MelGeneralizedLineSpectralPairsToSpectrum(int num_input_order, double alpha,
                                              double gamma,
                                              int num_output_order)
    : num_input_order_(num_input_order),
      alpha_(alpha),
      gamma_(gamma),
      num_output_order_(num_output_order),
      is_valid_(true) {
  if (num_input_order_ < 0 || !sptk::IsValidAlpha(alpha_) ||
      !sptk::IsValidGamma(gamma_) || num_output_order_ < 0) {
    is_valid_ = false;
  }
}

bool MelGeneralizedLineSpectralPairsToSpectrum::Run(
    const std::vector<double>& mel_generalized_line_spectral_pairs,
    std::vector<double>* spectrum) const {
  if (!is_valid_ ||
      mel_generalized_line_spectral_pairs.size() !=
          static_cast<std::size_t>(num_input_order_ + 1) ||
      NULL == spectrum) {
    return false;
  }

  // prepare memory
  const int output_length(num_output_order_ + 1);
  if (spectrum->size() != static_cast<std::size_t>(output_length)) {
    spectrum->resize(output_length);
  }

  // get values
  const double* input(&(mel_generalized_line_spectral_pairs[0]));
  double* output(&((*spectrum)[0]));

  // set value
  const bool is_odd(num_input_order_ % 2 == 1);
  const double c1(is_odd ? (num_input_order_ - 1) * sptk::kLogTwo
                         : num_input_order_ * sptk::kLogTwo);
  const double c2(0.5 / gamma_);
  const double delta(0 == num_output_order_ ? 0.0
                                            : sptk::kPi / num_output_order_);
  double omega(0.0);

  for (int j(0); j < output_length; ++j, omega += delta) {
    const double warped_omega(
        0.0 == alpha_ ? omega
                      : omega +
                            2.0 * std::atan(alpha_ * std::sin(omega) /
                                            (1.0 - alpha_ * std::cos(omega))));
    const double cos_omega(std::cos(warped_omega));

    double p(0.0);
    for (int i(2); i <= num_input_order_; i += 2) {
      p += 2.0 * sptk::FloorLog(std::fabs(cos_omega - std::cos(input[i])));
    }
    double q(0.0);
    for (int i(1); i <= num_input_order_; i += 2) {
      q += 2.0 * sptk::FloorLog(std::fabs(cos_omega - std::cos(input[i])));
    }

    if (is_odd) {
      p += 2.0 * sptk::FloorLog(std::sin(warped_omega));
    } else {
      p += 2.0 * sptk::FloorLog(std::sin(warped_omega * 0.5));
      q += 2.0 * sptk::FloorLog(std::cos(warped_omega * 0.5));
    }

    output[j] =
        sptk::FloorLog(input[0]) + c2 * (c1 + sptk::AddInLogSpace(p, q));
  }

  return true;
}

}  // namespace sptk
