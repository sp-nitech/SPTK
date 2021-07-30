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
//                1996-2021  Nagoya Institute of Technology          //
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

#include "SPTK/conversion/composite_sinusoidal_modeling_to_autocorrelation.h"

#include <cmath>    // std::cos
#include <cstddef>  // std::size_t

namespace sptk {

CompositeSinusoidalModelingToAutocorrelation::
    CompositeSinusoidalModelingToAutocorrelation(int num_sine_wave)
    : num_sine_wave_(num_sine_wave), is_valid_(true) {
  if (num_sine_wave_ <= 0) {
    is_valid_ = false;
    return;
  }
}

bool CompositeSinusoidalModelingToAutocorrelation::Run(
    const std::vector<double>& composite_sinusoidal_modeling,
    std::vector<double>* autocorrelation) const {
  // Check inputs.
  const int length(num_sine_wave_ * 2);
  if (!is_valid_ ||
      composite_sinusoidal_modeling.size() !=
          static_cast<std::size_t>(length) ||
      NULL == autocorrelation) {
    return false;
  }

  // Prepare memories.
  if (autocorrelation->size() != static_cast<std::size_t>(length)) {
    autocorrelation->resize(length);
  }

  const double* frequencies(&(composite_sinusoidal_modeling[0]));
  const double* intensities(&(composite_sinusoidal_modeling[num_sine_wave_]));
  double* v(&((*autocorrelation)[0]));

  // Calculate autocorrelation using Eq. (2).
  for (int l(0); l < length; ++l) {
    double sum(0.0);
    for (int i(0); i < num_sine_wave_; ++i) {
      sum += intensities[i] * std::cos(l * frequencies[i]);
    }
    v[l] = sum;
  }

  return true;
}

bool CompositeSinusoidalModelingToAutocorrelation::Run(
    std::vector<double>* input_and_output) const {
  if (NULL == input_and_output) return false;
  std::vector<double> input(*input_and_output);
  return Run(input, input_and_output);
}

}  // namespace sptk
