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

#include "SPTK/math/histogram_calculator.h"

#include <algorithm>  // std::fill
#include <cmath>      // std::floor
#include <cstddef>    // std::size_t

namespace sptk {

HistogramCalculator::HistogramCalculator(int length, int num_bin,
                                         double lower_bound, double upper_bound)
    : length_(length),
      num_bin_(num_bin),
      lower_bound_(lower_bound),
      upper_bound_(upper_bound),
      bin_width_((upper_bound_ - lower_bound_) / num_bin_),
      is_valid_(true) {
  if (length_ <= 0 || num_bin_ <= 0 || upper_bound_ <= lower_bound_) {
    is_valid_ = false;
  }
}

bool HistogramCalculator::Run(const std::vector<double>& data,
                              std::vector<double>* histogram) const {
  // check inputs
  if (!is_valid_ || data.size() != static_cast<std::size_t>(length_) ||
      NULL == histogram) {
    return false;
  }

  // prepare memory
  if (histogram->size() != static_cast<std::size_t>(num_bin_)) {
    histogram->resize(num_bin_);
  }

  // fill zero
  std::fill(histogram->begin(), histogram->end(), 0.0);

  const double* input(&(data[0]));
  double* output(&((*histogram)[0]));
  for (int i(0); i < length_; ++i) {
    if (lower_bound_ <= input[i] && input[i] < upper_bound_) {
      const int bin_index(std::floor((input[i] - lower_bound_) / bin_width_));
      ++output[bin_index];
    } else if (upper_bound_ == input[i]) {
      ++output[num_bin_ - 1];
    }
  }

  return true;
}

}  // namespace sptk
