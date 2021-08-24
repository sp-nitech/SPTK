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

#include "SPTK/check/line_spectral_pairs_stability_check.h"

#include <algorithm>  // std::copy
#include <cstddef>    // std::size_t

namespace {

const int kNumIteration(100);

}  // namespace

namespace sptk {

LineSpectralPairsStabilityCheck::LineSpectralPairsStabilityCheck(
    int num_order, double minimum_distance)
    : num_order_(num_order),
      minimum_distance_(minimum_distance),
      is_valid_(true) {
  if (num_order_ < 0 || minimum_distance_ < 0.0 ||
      sptk::kPi / (num_order_ + 1) < minimum_distance_) {
    is_valid_ = false;
    return;
  }
}

bool LineSpectralPairsStabilityCheck::Run(
    const std::vector<double>& line_spectral_pairs,
    std::vector<double>* modified_line_spectral_pairs, bool* is_stable) const {
  // Check inputs.
  if (!is_valid_ ||
      line_spectral_pairs.size() != static_cast<std::size_t>(num_order_ + 1) ||
      NULL == is_stable) {
    return false;
  }

  // Prepare memories.
  if (NULL != modified_line_spectral_pairs &&
      modified_line_spectral_pairs->size() !=
          static_cast<std::size_t>(num_order_ + 1)) {
    modified_line_spectral_pairs->resize(num_order_ + 1);
  }

  // Handle a special case.
  *is_stable = true;
  if (0 == num_order_) {
    if (NULL != modified_line_spectral_pairs) {
      (*modified_line_spectral_pairs)[0] = line_spectral_pairs[0];
    }
    return true;
  }

  // Check stability.
  {
    const double* input(&(line_spectral_pairs[0]));
    if (input[1] <= 0.0 || sptk::kPi <= input[num_order_]) {
      *is_stable = false;
    }
    for (int m(2); m <= num_order_; ++m) {
      if (input[m] <= input[m - 1]) {
        *is_stable = false;
        break;
      }
    }
  }

  // Modify line spectral pairs.
  if (NULL != modified_line_spectral_pairs) {
    std::copy(line_spectral_pairs.begin(), line_spectral_pairs.end(),
              modified_line_spectral_pairs->begin());
    if (!*is_stable || 0.0 < minimum_distance_) {
      const double lower_bound(minimum_distance_);
      const double upper_bound(sptk::kPi - minimum_distance_);
      double* output(&((*modified_line_spectral_pairs)[0]));
      for (int j(0); j < kNumIteration; ++j) {
        bool halt(true);
        for (int m(2); m <= num_order_; ++m) {
          const double distance(output[m] - output[m - 1]);
          if (distance < minimum_distance_) {
            const double step_size(0.5 * (minimum_distance_ - distance));
            output[m - 1] -= step_size;
            output[m] += step_size;
            halt = false;
          }
        }
        if (output[1] < lower_bound) {
          output[1] = lower_bound;
          halt = false;
        }
        if (upper_bound < output[num_order_]) {
          output[num_order_] = upper_bound;
          halt = false;
        }
        if (halt) {
          break;
        }
      }
    }
  }

  return true;
}

bool LineSpectralPairsStabilityCheck::Run(std::vector<double>* input_and_output,
                                          bool* is_stable) const {
  if (NULL == input_and_output) return false;
  return Run(*input_and_output, input_and_output, is_stable);
}

}  // namespace sptk
