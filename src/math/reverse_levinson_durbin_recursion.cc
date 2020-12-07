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
//                1996-2020  Nagoya Institute of Technology          //
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

#include <cstddef>  // std::size_t

namespace sptk {

ReverseLevinsonDurbinRecursion::ReverseLevinsonDurbinRecursion(int num_order)
    : num_order_(num_order), is_valid_(true) {
  if (num_order_ < 0) {
    is_valid_ = false;
    return;
  }
}

bool ReverseLevinsonDurbinRecursion::Run(
    const std::vector<double>& linear_predictive_coefficients,
    std::vector<double>* autocorrelation,
    ReverseLevinsonDurbinRecursion::Buffer* buffer) const {
  // Check inputs.
  const int length(num_order_ + 1);
  if (!is_valid_ ||
      linear_predictive_coefficients.size() !=
          static_cast<std::size_t>(length) ||
      NULL == autocorrelation || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  if (autocorrelation->size() != static_cast<std::size_t>(length)) {
    autocorrelation->resize(length);
  }
  if (buffer->u_.GetNumDimension() != length) {
    buffer->u_.Resize(length);
  }
  if (buffer->e_.size() != static_cast<std::size_t>(length)) {
    buffer->e_.resize(length);
  }

  const double* a(&(linear_predictive_coefficients[0]));
  double* r(&((*autocorrelation)[0]));
  double* e(&(buffer->e_[0]));

  for (int j(0); j <= num_order_; ++j) {
    buffer->u_[j][j] = 1.0;
  }
  for (int j(0); j < num_order_; ++j) {
    buffer->u_[num_order_][j] = a[num_order_ - j];
  }
  e[num_order_] = a[0] * a[0];

  for (int i(num_order_ - 1); 0 <= i; --i) {
    double* u(&(buffer->u_[i + 1][0]));
    const double t(1.0 / (1.0 - u[0] * u[0]));
    for (int j(0); j < i; ++j) {
      buffer->u_[i][j] = (u[j + 1] - u[0] * u[i - j]) * t;
    }
    e[i] = e[i + 1] * t;
  }

  r[0] = e[0];
  for (int i(1); i <= num_order_; ++i) {
    double sum(0.0);
    for (int j(1); j < i; ++j) {
      sum += buffer->u_[i - 1][i - j - 1] * r[i - j];
    }
    r[i] = -(sum + buffer->u_[i][0] * e[i - 1]);
  }

  return true;
}

bool ReverseLevinsonDurbinRecursion::Run(
    std::vector<double>* input_and_output,
    ReverseLevinsonDurbinRecursion::Buffer* buffer) const {
  if (NULL == input_and_output) return false;
  return Run(*input_and_output, input_and_output, buffer);
}

}  // namespace sptk
