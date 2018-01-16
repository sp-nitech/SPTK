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

#include "SPTK/converter/composite_sinusoidal_modeling_to_autocorrelation.h"

#include <cmath>    // std::cos
#include <cstddef>  // std::size_t

namespace sptk {

CompositeSinusoidalModelingToAutocorrelation::
    CompositeSinusoidalModelingToAutocorrelation(int num_sine_waves)
    : num_sine_waves_(num_sine_waves), is_valid_(true) {
  if (num_sine_waves_ <= 0) {
    is_valid_ = false;
  }
}

bool CompositeSinusoidalModelingToAutocorrelation::Run(
    const std::vector<double>& composite_sinusoidal_modeling,
    std::vector<double>* autocorrelation) const {
  // check inputs
  const int output_length(num_sine_waves_ * 2);
  if (!is_valid_ ||
      composite_sinusoidal_modeling.size() !=
          static_cast<std::size_t>(output_length) ||
      NULL == autocorrelation) {
    return false;
  }

  // prepare memory
  if (autocorrelation->size() < static_cast<std::size_t>(output_length)) {
    autocorrelation->resize(output_length);
  }

  // get value
  const double* frequencies(&(composite_sinusoidal_modeling[0]));
  const double* intensities(&(composite_sinusoidal_modeling[num_sine_waves_]));
  double* output(&((*autocorrelation)[0]));

  for (int l(0); l < output_length; ++l) {
    double sum(0.0);
    for (int i(0); i < num_sine_waves_; ++i) {
      sum += intensities[i] * std::cos(l * frequencies[i]);
    }
    output[l] = sum;
  }

  return true;
}

}  // namespace sptk
