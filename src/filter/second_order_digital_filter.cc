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

#include "SPTK/filter/second_order_digital_filter.h"

#include <cmath>   // std::cos, std::exp
#include <vector>  // std::vector

namespace {

std::vector<double> GetFilterCoefficients(double frequency, double bandwidth,
                                          double sampling_rate) {
  const double radius(std::exp(-sptk::kPi * bandwidth / sampling_rate));
  const double angle(2.0 * sptk::kPi * frequency / sampling_rate);
  return {1.0, -2.0 * radius * std::cos(angle), radius * radius};
}

}  // namespace

namespace sptk {

SecondOrderDigitalFilter::SecondOrderDigitalFilter(FilterType filter_type,
                                                   double frequency,
                                                   double bandwidth,
                                                   double sampling_rate)
    : filter_((kPole == filter_type)
                  ? GetFilterCoefficients(frequency, bandwidth, sampling_rate)
                  : std::vector<double>({1.0}),
              (kPole == filter_type)
                  ? std::vector<double>({1.0})
                  : GetFilterCoefficients(frequency, bandwidth, sampling_rate)),
      is_valid_(true) {
  const double nyquist_frequency(0.5 * sampling_rate);
  if (frequency <= 0.0 || nyquist_frequency <= frequency || bandwidth <= 0.0) {
    is_valid_ = false;
    return;
  }
}

SecondOrderDigitalFilter::SecondOrderDigitalFilter(double pole_frequency,
                                                   double pole_bandwidth,
                                                   double zero_frequency,
                                                   double zero_bandwidth,
                                                   double sampling_rate)
    : filter_(
          GetFilterCoefficients(pole_frequency, pole_bandwidth, sampling_rate),
          GetFilterCoefficients(zero_frequency, zero_bandwidth, sampling_rate)),
      is_valid_(true) {
  const double nyquist_frequency(0.5 * sampling_rate);
  if (pole_frequency <= 0.0 || nyquist_frequency <= pole_frequency ||
      pole_bandwidth <= 0.0 || zero_frequency <= 0.0 ||
      nyquist_frequency <= zero_frequency || zero_bandwidth <= 0.0 ||
      !filter_.IsValid()) {
    is_valid_ = false;
    return;
  }
}

bool SecondOrderDigitalFilter::Run(
    double input, double* output,
    SecondOrderDigitalFilter::Buffer* buffer) const {
  if (!is_valid_ || NULL == output || NULL == buffer) {
    return false;
  }
  return filter_.Run(input, output, &(buffer->buffer));
}

bool SecondOrderDigitalFilter::Run(
    double* input_and_output, SecondOrderDigitalFilter::Buffer* buffer) const {
  if (NULL == input_and_output) return false;
  return Run(*input_and_output, input_and_output, buffer);
}

}  // namespace sptk
