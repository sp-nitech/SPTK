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

#include "SPTK/math/reverse_levinson_durbin_recursion.h"

#include <cmath>    // std::fabs
#include <cstddef>  // std::size_t

namespace sptk {

ReverseLevinsonDurbinRecursion::ReverseLevinsonDurbinRecursion(int num_order,
                                                               double epsilon)
    : num_order_(num_order), epsilon_(epsilon), is_valid_(true) {
  if (num_order_ < 0 || epsilon_ < 0.0) {
    is_valid_ = false;
  }
}

bool ReverseLevinsonDurbinRecursion::Run(
    const std::vector<double>& linear_predictive_coefficients,
    std::vector<double>* autocorrelation_sequence,
    ReverseLevinsonDurbinRecursion::Buffer* buffer) const {
  // check inputs
  const int length(num_order_ + 1);
  if (!is_valid_ ||
      linear_predictive_coefficients.size() !=
          static_cast<std::size_t>(length) ||
      NULL == autocorrelation_sequence || NULL == buffer) {
    return false;
  }

  // prepare memories
  if (autocorrelation_sequence->size() < static_cast<std::size_t>(length)) {
    autocorrelation_sequence->resize(length);
  }

  // prepare buffer
  if (buffer->u_.size() < static_cast<std::size_t>(length)) {
    buffer->u_.resize(length);
    for (int i(0); i < length; ++i) {
      buffer->u_[i].resize(i + 1);
    }
  }
  if (buffer->e_.size() < static_cast<std::size_t>(length)) {
    buffer->e_.resize(length);
  }

  // get values
  const double* input(&(linear_predictive_coefficients[0]));
  double* output(&((*autocorrelation_sequence)[0]));
  double* e(&(buffer->e_[0]));

  if (0 == num_order_) {
    output[0] = input[0] * input[0];
    return true;
  }

  for (int j(0); j <= num_order_; ++j) {
    buffer->u_[j][j] = 1.0;
  }

  for (int j(0); j < num_order_; ++j) {
    buffer->u_[num_order_][j] = input[num_order_ - j];
  }
  e[num_order_] = input[0] * input[0];

  for (int i(num_order_ - 1); 0 < i; --i) {
    const double rmd(1.0 - buffer->u_[i + 1][0] * buffer->u_[i + 1][0]);
    if (std::fabs(rmd) <= epsilon_) {
      return false;
    }
    const double inverse_rmd(1.0 / rmd);
    for (int j(0); j < i; ++j) {
      buffer->u_[i][i - j - 1] =
          (buffer->u_[i + 1][i - j] -
           buffer->u_[i + 1][0] * buffer->u_[i + 1][j + 1]) *
          inverse_rmd;
    }
    e[i] = e[i + 1] * inverse_rmd;
  }
  e[0] = e[1] / (1.0 - buffer->u_[1][0] * buffer->u_[1][0]);

  output[0] = e[0];
  for (int i(1); i <= num_order_; ++i) {
    double sum(0.0);
    for (int j(1); j < i; ++j) {
      sum += buffer->u_[i - 1][i - j - 1] * output[i - j];
    }
    output[i] = -sum - buffer->u_[i][0] * e[i - 1];
  }

  return true;
}

}  // namespace sptk
