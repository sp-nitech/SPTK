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

#include "SPTK/math/vandermonde_system_solver.h"

#include <algorithm>  // std::fill
#include <cstddef>    // std::size_t

namespace sptk {

VandermondeSystemSolver::VandermondeSystemSolver(int num_order)
    : num_order_(num_order), is_valid_(true) {
  if (num_order_ < 0) {
    is_valid_ = false;
  }
}

bool VandermondeSystemSolver::Run(
    const std::vector<double>& coefficient_vector,
    const std::vector<double>& constant_vector,
    std::vector<double>* solution_vector,
    VandermondeSystemSolver::Buffer* buffer) const {
  // check inputs
  const int length(num_order_ + 1);
  if (!is_valid_ ||
      coefficient_vector.size() != static_cast<std::size_t>(length) ||
      constant_vector.size() != static_cast<std::size_t>(length) ||
      NULL == solution_vector || NULL == buffer) {
    return false;
  }

  // prepare memories
  if (solution_vector->size() != static_cast<std::size_t>(length)) {
    solution_vector->resize(length);
  }

  // prepare buffer
  if (buffer->d_.size() != static_cast<std::size_t>(length)) {
    buffer->d_.resize(length);
  }

  // get values
  const double* a(&coefficient_vector[0]);
  const double* b(&constant_vector[0]);
  double* x(&((*solution_vector)[0]));
  double* d(&buffer->d_[0]);

  std::fill(buffer->d_.begin(), buffer->d_.end(), 0.0);
  for (int i(0); i < length; ++i) {
    for (int j(i); 0 < j; --j) {
      d[j] -= a[i] * d[j - 1];
    }
    d[0] -= a[i];
  }

  for (int i(0); i <= num_order_; ++i) {
    double tmp(1.0);
    double numerator(b[num_order_]);
    double denominator(1.0);
    for (int j(0); j < num_order_; ++j) {
      tmp = d[j] + a[i] * tmp;
      numerator = numerator + b[num_order_ - j - 1] * tmp;
      denominator = denominator * a[i] + tmp;
    }

    if (0.0 == denominator) {
      return false;
    }
    x[i] = numerator / denominator;
  }

  return true;
}

}  // namespace sptk
