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

#include "SPTK/analysis/goertzel_analysis.h"

#include <cmath>    // std::round
#include <cstddef>  // std::size_t
#include <vector>   // std::vector

namespace sptk {

GoertzelAnalysis::GoertzelAnalysis(double sampling_rate,
                                   const std::vector<double>& frequencies,
                                   int fft_length)
    : is_valid_(true) {
  if (sampling_rate <= 0.0 || frequencies.empty() || fft_length <= 0) {
    is_valid_ = false;
    return;
  }

  const int table_size(frequencies.size());
  sine_table_.resize(table_size);
  cosine_table_.resize(table_size);
  for (int k(0); k < table_size; ++k) {
    if (frequencies[k] < 0.0 || 0.5 * sampling_rate <= frequencies[k]) {
      is_valid_ = false;
      break;
    }
    const double n(std::round(fft_length * frequencies[k] / sampling_rate));
    const double omega(sptk::kTwoPi * n / fft_length);
    sine_table_[k] = std::sin(omega);
    cosine_table_[k] = std::cos(omega);
  }
}

bool GoertzelAnalysis::Run(const std::vector<double>& signals,
                           std::vector<double>* real_part,
                           std::vector<double>* imag_part) const {
  // Check inputs.
  if (!is_valid_ || NULL == real_part || NULL == imag_part) {
    return false;
  }

  // Prepare memories.
  const int table_size(sine_table_.size());
  if (real_part->size() != static_cast<std::size_t>(table_size)) {
    real_part->resize(table_size);
  }
  if (imag_part->size() != static_cast<std::size_t>(table_size)) {
    imag_part->resize(table_size);
  }

  const int signal_size(signals.size());
  const double* x(&(signals[0]));
  double* real(&((*real_part)[0]));
  double* imag(&((*imag_part)[0]));

  for (int k(0); k < table_size; ++k) {
    // Perform second-order IIR filtering.
    const double c(2.0 * cosine_table_[k]);
    double s1(0.0), s2(0.0);
    {
      double s0;
      for (int t(0); t < signal_size; ++t) {
        s0 = x[t] + c * s1 - s2;
        s2 = s1;
        s1 = s0;
      }
    }
    // Perform first-order FIR filtering.
    real[k] = s1 * cosine_table_[k] - s2;
    imag[k] = s1 * sine_table_[k];
  }

  return true;
}

}  // namespace sptk
