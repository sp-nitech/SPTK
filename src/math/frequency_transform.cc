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

#include "SPTK/math/frequency_transform.h"

#include <algorithm>  // std::copy, std::fill
#include <cstddef>    // std::size_t

namespace sptk {

FrequencyTransform::FrequencyTransform(int num_input_order,
                                       int num_output_order, double alpha)
    : num_input_order_(num_input_order),
      num_output_order_(num_output_order),
      alpha_(alpha),
      is_valid_(true) {
  if (num_input_order_ < 0 || num_output_order_ < 0) {
    is_valid_ = false;
  }
}

bool FrequencyTransform::Run(const std::vector<double>& minimum_phase_sequence,
                             std::vector<double>* warped_sequence,
                             FrequencyTransform::Buffer* buffer) const {
  // check inputs
  const int input_length(num_input_order_ + 1);
  if (!is_valid_ ||
      minimum_phase_sequence.size() != static_cast<std::size_t>(input_length) ||
      NULL == warped_sequence || NULL == buffer) {
    return false;
  }

  // prepare memory
  const int output_length(num_output_order_ + 1);
  if (warped_sequence->size() != static_cast<std::size_t>(output_length)) {
    warped_sequence->resize(output_length);
  }

  // handle specific case
  if (0.0 == alpha_) {
    if (num_input_order_ < num_output_order_) {
      std::copy(minimum_phase_sequence.begin(), minimum_phase_sequence.end(),
                warped_sequence->begin());
      std::fill(warped_sequence->begin() + input_length, warped_sequence->end(),
                0.0);
    } else {
      std::copy(minimum_phase_sequence.begin(),
                minimum_phase_sequence.begin() + output_length,
                warped_sequence->begin());
    }
    return true;
  }

  // prepare buffer
  if (buffer->d_.size() != static_cast<std::size_t>(output_length)) {
    buffer->d_.resize(output_length);
  }
  if (buffer->g_.size() != static_cast<std::size_t>(output_length)) {
    buffer->g_.resize(output_length);
  }

  // get values
  const double* input(&(minimum_phase_sequence[0]));
  double* d(&buffer->d_[0]);
  double* g(&buffer->g_[0]);

  // set value
  const double beta(1.0 - alpha_ * alpha_);

  // fill zero
  std::fill(buffer->g_.begin(), buffer->g_.end(), 0.0);

  // transform
  for (int i(num_input_order_); 0 <= i; --i) {
    d[0] = g[0];
    g[0] = input[i] + alpha_ * d[0];
    if (1 <= num_output_order_) {
      d[1] = g[1];
      g[1] = beta * d[0] + alpha_ * d[1];
    }
    for (int j(2); j <= num_output_order_; ++j) {
      d[j] = g[j];
      g[j] = d[j - 1] + alpha_ * (d[j] - g[j - 1]);
    }
  }

  // save results
  std::copy(buffer->g_.begin(), buffer->g_.end(), warped_sequence->begin());

  return true;
}

}  // namespace sptk
