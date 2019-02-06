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

#include "SPTK/utils/line_spectral_pairs_stability_check.h"

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
  }
}

bool LineSpectralPairsStabilityCheck::Run(
    const std::vector<double>& line_spectral_pairs,
    std::vector<double>* modified_line_spectral_pairs, bool* is_stable) const {
  if (!is_valid_ ||
      line_spectral_pairs.size() != static_cast<std::size_t>(num_order_ + 1) ||
      NULL == is_stable) {
    return false;
  }

  // prepare memory
  if (NULL != modified_line_spectral_pairs &&
      modified_line_spectral_pairs->size() !=
          static_cast<std::size_t>(num_order_ + 1)) {
    modified_line_spectral_pairs->resize(num_order_ + 1);
  }

  *is_stable = true;
  if (0 == num_order_) {
    if (NULL != modified_line_spectral_pairs) {
      (*modified_line_spectral_pairs)[0] = line_spectral_pairs[0];
    }
    return true;
  }

  // check stability
  const double* input(&(line_spectral_pairs[1]));
  for (int i(1); i < num_order_; ++i) {
    if (input[i] <= input[i - 1]) {
      *is_stable = false;
      break;
    }
  }
  if (input[0] <= 0.0 || sptk::kPi <= input[num_order_ - 1]) {
    *is_stable = false;
  }

  // modify line spectral pairs
  if (NULL != modified_line_spectral_pairs) {
    std::copy(line_spectral_pairs.begin(), line_spectral_pairs.end(),
              modified_line_spectral_pairs->begin());
    if (!*is_stable || 0.0 < minimum_distance_) {
      const double lower_bound(minimum_distance_);
      const double upper_bound(sptk::kPi - minimum_distance_);
      double* output(&((*modified_line_spectral_pairs)[1]));
      bool halt(false);
      for (int j(0); j < kNumIteration && !halt; ++j) {
        halt = true;
        for (int i(1); i < num_order_; ++i) {
          const double distance(output[i] - output[i - 1]);
          if (distance < minimum_distance_) {
            const double step_size(0.5 * (minimum_distance_ - distance));
            output[i - 1] -= step_size;
            output[i] += step_size;
            halt = false;
          }
        }
        if (output[0] < lower_bound) {
          output[0] = lower_bound;
          halt = false;
        }
        if (upper_bound < output[num_order_ - 1]) {
          output[num_order_ - 1] = upper_bound;
          halt = false;
        }
      }
    }
  }

  return true;
}

}  // namespace sptk
