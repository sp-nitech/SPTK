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

#include "SPTK/math/distance_calculator.h"

#include <cmath>    // std::fabs, std::log, std::sqrt
#include <cstddef>  // std::size_t

namespace sptk {

DistanceCalculator::DistanceCalculator(int num_order,
                                       DistanceMetrics distance_metric)
    : num_order_(num_order),
      distance_metric_(distance_metric),
      is_valid_(true) {
  if (num_order_ < 0 || kNumMetrics == distance_metric_) {
    is_valid_ = false;
  }
}

bool DistanceCalculator::Run(const std::vector<double>& vector1,
                             const std::vector<double>& vector2,
                             double* distance) const {
  // check inputs
  if (!is_valid_ ||
      vector1.size() != static_cast<std::size_t>(num_order_ + 1) ||
      vector2.size() != static_cast<std::size_t>(num_order_ + 1) ||
      NULL == distance) {
    return false;
  }

  // get values
  const double* x(&(vector1[0]));
  const double* y(&(vector2[0]));

  double sum(0.0);

  switch (distance_metric_) {
    case kManhattan: {
      for (int i(0); i <= num_order_; ++i) {
        const double diff(x[i] - y[i]);
        sum += std::fabs(diff);
      }
      break;
    }
    case kEuclidean: {
      for (int i(0); i <= num_order_; ++i) {
        const double diff(x[i] - y[i]);
        sum += diff * diff;
      }
      sum = std::sqrt(sum);
      break;
    }
    case kSquaredEuclidean: {
      for (int i(0); i <= num_order_; ++i) {
        const double diff(x[i] - y[i]);
        sum += diff * diff;
      }
      break;
    }
    case kSymmetricKullbackLeibler: {
      for (int i(0); i <= num_order_; ++i) {
        if (x[i] <= 0.0 || y[i] <= 0.0) return false;
        const double diff(x[i] - y[i]);
        const double log_diff(std::log(x[i]) - std::log(y[i]));
        sum += diff * log_diff;
      }
      break;
    }
    default: { return false; }
  }

  *distance = sum;

  return true;
}

}  // namespace sptk
