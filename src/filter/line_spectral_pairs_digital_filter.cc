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

#include "SPTK/filter/line_spectral_pairs_digital_filter.h"

#include <algorithm>  // std::fill, std::transform
#include <cmath>      // std::cos
#include <cstddef>    // std::size_t

namespace sptk {

LineSpectralPairsDigitalFilter::LineSpectralPairsDigitalFilter(
    int num_filter_order)
    : num_filter_order_(num_filter_order), is_valid_(true) {
  if (num_filter_order_ < 0) {
    is_valid_ = false;
    return;
  }
}

bool LineSpectralPairsDigitalFilter::Run(
    const std::vector<double>& filter_coefficients, double filter_input,
    double* filter_output,
    LineSpectralPairsDigitalFilter::Buffer* buffer) const {
  // Check inputs.
  if (!is_valid_ ||
      filter_coefficients.size() !=
          static_cast<std::size_t>(num_filter_order_ + 1) ||
      NULL == filter_output || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  if (buffer->d1_.size() != static_cast<std::size_t>(num_filter_order_ + 1)) {
    buffer->d1_.resize(num_filter_order_ + 1);
    std::fill(buffer->d1_.begin(), buffer->d1_.end(), 0.0);
  }
  if (buffer->d2_.size() != static_cast<std::size_t>(num_filter_order_ + 1)) {
    buffer->d2_.resize(num_filter_order_ + 1);
    std::fill(buffer->d2_.begin(), buffer->d2_.end(), 0.0);
  }
  if (buffer->ab_.size() != static_cast<std::size_t>(num_filter_order_)) {
    buffer->ab_.resize(num_filter_order_);
  }

  const double gained_input(filter_input * filter_coefficients[0]);
  if (0 == num_filter_order_) {
    *filter_output = gained_input;
    return true;
  }

  std::transform(filter_coefficients.begin() + 1, filter_coefficients.end(),
                 buffer->ab_.begin(),
                 [](double w) { return -2.0 * std::cos(w); });

  const double* ab(&(buffer->ab_[0]));
  double* d1(&buffer->d1_[0]);
  double* d2(&buffer->d2_[0]);

  // Apply LSP synthesis filter.
  double sum(gained_input);
  {
    double x1(d1[0]);
    double x2(d2[0]);
    for (int i(1); i < num_filter_order_; i += 2) {
      d1[i] += x1 * ab[i - 1];
      d2[i] += x2 * ab[i];
      d1[i + 1] += x1;
      d2[i + 1] += x2;
      x1 = d1[i + 1];
      x2 = d2[i + 1];
      sum += d1[i] + d2[i];
    }
    if (!sptk::IsEven(num_filter_order_)) {
      d1[num_filter_order_] += x1 * ab[num_filter_order_ - 1];
    }
    sum += d1[num_filter_order_] - d2[num_filter_order_];
  }

  // Save result.
  *filter_output = sum;

  // Shift stored signals.
  for (int i(num_filter_order_); 0 < i; --i) {
    d1[i] = d1[i - 1];
    d2[i] = d2[i - 1];
  }
  const double delayed_output(-0.5 * sum);
  d1[0] = delayed_output;
  d2[0] = delayed_output;

  return true;
}

bool LineSpectralPairsDigitalFilter::Run(
    const std::vector<double>& filter_coefficients, double* input_and_output,
    LineSpectralPairsDigitalFilter::Buffer* buffer) const {
  if (NULL == input_and_output) return false;
  return Run(filter_coefficients, *input_and_output, input_and_output, buffer);
}

}  // namespace sptk
