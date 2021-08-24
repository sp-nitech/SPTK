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

#include "SPTK/analysis/mel_frequency_cepstral_coefficients_analysis.h"

#include <algorithm>  // std::transform
#include <cmath>      // std::sin, std::sqrt
#include <cstddef>    // std::size_t

namespace sptk {

MelFrequencyCepstralCoefficientsAnalysis::
    MelFrequencyCepstralCoefficientsAnalysis(
        int fft_length, int num_channel, int num_order,
        int liftering_coefficient, double sampling_rate,
        double lowest_frequency, double highest_frequency, double floor)
    : num_order_(num_order),
      liftering_coefficient_(liftering_coefficient),
      mel_filter_bank_analysis_(fft_length, num_channel, sampling_rate,
                                lowest_frequency, highest_frequency, floor,
                                false),
      discrete_cosine_transform_(num_channel),
      is_valid_(true) {
  if (num_channel <= num_order_ || liftering_coefficient_ <= 0 ||
      !mel_filter_bank_analysis_.IsValid() ||
      !discrete_cosine_transform_.IsValid()) {
    is_valid_ = false;
    return;
  }

  cepstal_weights_.resize(num_order_);
  double* w(&(cepstal_weights_[0]));
  for (int m(0); m < num_order_; ++m) {
    const double theta(sptk::kPi * (m + 1) / liftering_coefficient_);
    w[m] = 1.0 + 0.5 * liftering_coefficient_ * std::sin(theta);
  }
}

bool MelFrequencyCepstralCoefficientsAnalysis::Run(
    const std::vector<double>& power_spectrum, std::vector<double>* mfcc,
    double* energy,
    MelFrequencyCepstralCoefficientsAnalysis::Buffer* buffer) const {
  // Check inputs.
  if (!is_valid_ || NULL == mfcc || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  const int num_channel(GetNumChannel());
  if (mfcc->size() != static_cast<std::size_t>(num_order_ + 1)) {
    mfcc->resize(num_order_ + 1);
  }
  if (buffer->imag_part_input_.size() !=
      static_cast<std::size_t>(num_channel)) {
    buffer->imag_part_input_.resize(num_channel);
  }

  if (!mel_filter_bank_analysis_.Run(power_spectrum,
                                     &buffer->filter_bank_output_, energy)) {
    return false;
  }

  if (!discrete_cosine_transform_.Run(
          buffer->filter_bank_output_, buffer->imag_part_input_,
          &buffer->cepstrum_, &buffer->imag_part_output_,
          &buffer->buffer_for_discrete_cosine_transform_)) {
    return false;
  }

  // C: Number of channels.
  // F: Mel-filter-bank outputs.
  // C0 = \sum F(j) * sqrt(2 / C)
  //    = (buffer->cepstrum_[0] / sqrt(C)) * sqrt(2 / C)
  //    = buffer->cepstrum_[0] * sqrt(2).
  (*mfcc)[0] = buffer->cepstrum_[0] * std::sqrt(2.0);

  // Lifter.
  std::transform(buffer->cepstrum_.begin() + 1,
                 buffer->cepstrum_.begin() + 1 + num_order_,
                 cepstal_weights_.begin(), mfcc->begin() + 1,
                 [](double c, double w) { return c * w; });

  return true;
}

}  // namespace sptk
