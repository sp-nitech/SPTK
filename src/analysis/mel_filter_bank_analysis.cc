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

#include "SPTK/analysis/mel_filter_bank_analysis.h"

#include <algorithm>  // std::fill, std::max, std::min
#include <cmath>      // std::log, std::sqrt
#include <cstddef>    // std::size_t
#include <numeric>    // std::accumulate

namespace {

// Note that HTK use 1127 instead of 1127.01048.
double HzToMel(double hz) {
  // return 1127.01048 * std::log(hz / 700.0 + 1.0);
  return 1127 * std::log(hz / 700.0 + 1.0);
}

double SampleMel(int index, int fft_length, double sampling_rate) {
  const double hz(sampling_rate * index / fft_length);
  return HzToMel(hz);
}

}  // namespace

namespace sptk {

MelFilterBankAnalysis::MelFilterBankAnalysis(int fft_length, int num_channel,
                                             double sampling_rate,
                                             double lowest_frequency,
                                             double highest_frequency,
                                             double floor, bool use_power)
    : fft_length_(fft_length),
      num_channel_(num_channel),
      floor_(floor),
      use_power_(use_power),
      is_valid_(true) {
  if (fft_length_ <= 0 || 1 == fft_length_ % 2 || num_channel_ <= 0 ||
      sampling_rate <= 0.0 || sampling_rate / 2 < highest_frequency ||
      lowest_frequency < 0.0 || highest_frequency <= lowest_frequency ||
      floor_ <= 0.0) {
    is_valid_ = false;
    return;
  }

  {
    const int min(1);
    const int index(static_cast<int>(
        (lowest_frequency / sampling_rate * fft_length_) + 1.5));
    lower_bin_index_ = std::max(min, index);
  }

  {
    const int max(fft_length_ / 2);
    const int index(static_cast<int>(
        (highest_frequency / sampling_rate * fft_length_) + 0.5));
    upper_bin_index_ = std::min(max, index);
  }

  const double mel_low(HzToMel(lowest_frequency));
  const double mel_high(HzToMel(highest_frequency));

  // Create vector of filter-bank center frequencies.
  std::vector<double> center_frequencies(num_channel_ + 1);
  double* cf(&(center_frequencies[0]));
  {
    const double diff(mel_high - mel_low);
    for (int m(0); m <= num_channel_; ++m) {
      cf[m] = diff * (m + 1) / (num_channel_ + 1) + mel_low;
    }
  }

  // Create lower channel map.
  channel_indices_.resize(fft_length_ / 2, -1);
  int* map(&(channel_indices_[0]));
  {
    for (int k(lower_bin_index_), m(0); k < upper_bin_index_; ++k) {
      const double mel_k(SampleMel(k, fft_length_, sampling_rate));
      while (cf[m] < mel_k && m <= num_channel_) ++m;
      map[k] = m;
    }
  }

  // Create vector of lower channel weights.
  channel_weights_.resize(fft_length_ / 2);
  double* w(&(channel_weights_[0]));
  for (int k(lower_bin_index_); k < upper_bin_index_; ++k) {
    const double mel_k(SampleMel(k, fft_length_, sampling_rate));
    const int m(map[k]);
    if (0 < m) {
      w[k] = (cf[m] - mel_k) / (cf[m] - cf[m - 1]);
    } else {
      w[k] = (cf[0] - mel_k) / (cf[0] - mel_low);
    }
  }
}

bool MelFilterBankAnalysis::Run(const std::vector<double>& power_spectrum,
                                std::vector<double>* filter_bank_output,
                                double* energy) const {
  // Check inputs.
  if (!is_valid_ ||
      power_spectrum.size() != static_cast<std::size_t>(fft_length_ / 2 + 1) ||
      NULL == filter_bank_output) {
    return false;
  }

  // Prepare memories.
  if (filter_bank_output->size() != static_cast<std::size_t>(num_channel_)) {
    filter_bank_output->resize(num_channel_);
  }

  std::fill(filter_bank_output->begin(), filter_bank_output->end(), 0.0);

  // Apply mel-filter-banks.
  const int* map(&(channel_indices_[0]));
  const double* w(&(channel_weights_[0]));
  const double* input(&(power_spectrum[0]));
  double* output(&((*filter_bank_output)[0]));
  for (int k(lower_bin_index_); k < upper_bin_index_; ++k) {
    const int m(map[k]);
    const double x(use_power_ ? input[k] : std::sqrt(input[k]));
    if (0 < m) {
      output[m - 1] += x * w[k];
    }
    if (m < num_channel_) {
      output[m] += x * (1.0 - w[k]);
    }
  }

  // Apply logarithm function.
  for (int m(0); m < num_channel_; ++m) {
    if (output[m] < floor_) output[m] = floor_;
    output[m] = std::log(output[m]);
  }

  if (NULL != energy) {
    const double sum(
        std::accumulate(power_spectrum.begin() + 1, power_spectrum.end() - 1,
                        power_spectrum.front() + power_spectrum.back(),
                        [](double a, double x) { return a + 2.0 * x; }));
    *energy = std::log(sum / fft_length_);
  }

  return true;
}

}  // namespace sptk
