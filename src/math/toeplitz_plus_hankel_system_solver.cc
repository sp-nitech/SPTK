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

#include "SPTK/math/toeplitz_plus_hankel_system_solver.h"

#include <cmath>    // std::fabs
#include <cstddef>  // std::size_t

namespace {

const double kMinimumValueOfDeterminant(1e-6);

bool Invert2dMatrix(const sptk::Matrix& x, sptk::Matrix* y) {
  const double determinant(x[0][0] * x[1][1] - x[0][1] * x[1][0]);
  if (std::fabs(determinant) < kMinimumValueOfDeterminant) {
    return false;
  }

  const double inverse_of_determinant(1.0 / determinant);
  (*y)[0][0] = x[1][1] * inverse_of_determinant;
  (*y)[1][1] = x[0][0] * inverse_of_determinant;
  (*y)[0][1] = -x[0][1] * inverse_of_determinant;
  (*y)[1][0] = -x[1][0] * inverse_of_determinant;
  return true;
}

void PutBar(int i, const sptk::Matrix& x, sptk::Matrix* y) {
  (*y)[0][0] = x[i][0];
  (*y)[1][0] = x[x.GetNumRow() - 1 - i][0];
}

void CrossTranspose(const sptk::Matrix& x, sptk::Matrix* y) {
  (*y)[0][0] = x[1][1];
  (*y)[1][1] = x[0][0];
  (*y)[0][1] = x[1][0];
  (*y)[1][0] = x[0][1];
}

}  // namespace

namespace sptk {

ToeplitzPlusHankelSystemSolver::ToeplitzPlusHankelSystemSolver(
    int num_order, bool coefficients_modification)
    : num_order_(num_order),
      coefficients_modification_(coefficients_modification),
      is_valid_(true) {
  if (num_order_ < 0) {
    is_valid_ = false;
  }
}

// This function solves the Toeplitz-plus-Hankel system given the coefficient
// vector of the Toeplitz matrix {t_ij}
//   [ t(1, n), t(1, n-1), ..., t(1, 1), t(2, 1), ..., t(n, 1) ],
// the coefficient vector of the Hankel matrix {h_ij}
//   [ h(1, 1), h(1, 2), ..., h(1, n), h(2, n), ..., h(n, n) ],
// and the constant vector
//   [ b(1), b(2), ..., b(n) ].
// The implementation and the name of variables are based on the paper written
// by Merchant and Parks.
//
// G. Merchant and T. Parks, ``Efficient solution of a Toeplitz-plus-Hankel
// coefficient matrix system of equations,'' IEEE Transactions on Acoustics,
// Speech, and Signal Processing, vol. 30, no. 1, pp. 40--44, 1982.
bool ToeplitzPlusHankelSystemSolver::Run(
    const std::vector<double>& toeplitz_coefficient_vector,
    const std::vector<double>& hankel_coefficient_vector,
    const std::vector<double>& constant_vector,
    std::vector<double>* solution_vector,
    ToeplitzPlusHankelSystemSolver::Buffer* buffer) const {
  // check inputs
  const int length(num_order_ + 1);
  if (!is_valid_ ||
      toeplitz_coefficient_vector.size() !=
          static_cast<std::size_t>(2 * length - 1) ||
      hankel_coefficient_vector.size() !=
          static_cast<std::size_t>(2 * length - 1) ||
      constant_vector.size() != static_cast<std::size_t>(length) ||
      NULL == solution_vector || NULL == buffer) {
    return false;
  }

  // prepare memories
  if (solution_vector->size() != static_cast<std::size_t>(length)) {
    solution_vector->resize(length);
  }
  if (buffer->r_.size() != static_cast<std::size_t>(length)) {
    buffer->r_.resize(length);
    for (int i(0); i < length; ++i) {
      buffer->r_[i].Resize(2, 2);
    }
  }
  if (buffer->x_.size() != static_cast<std::size_t>(length)) {
    buffer->x_.resize(length);
    for (int i(0); i < length; ++i) {
      buffer->x_[i].Resize(2, 2);
    }
  }
  if (buffer->prev_x_.size() != static_cast<std::size_t>(length)) {
    buffer->prev_x_.resize(length);
    for (int i(1); i < length; ++i) {
      buffer->prev_x_[i].Resize(2, 2);
    }
  }
  if (buffer->p_.size() != static_cast<std::size_t>(length)) {
    buffer->p_.resize(length);
    for (int i(0); i < length; ++i) {
      buffer->p_[i].Resize(2, 1);
    }
  }
  if (buffer->b_.GetNumRow() != length || buffer->b_.GetNumColumn() != 1) {
    buffer->b_.Resize(length, 1);
  }

  // Step 0)
  {
    // Set R
    const double* t(&(toeplitz_coefficient_vector[0]));
    const double* h(&(hankel_coefficient_vector[0]));
    for (int i(0); i < length; ++i) {
      buffer->r_[i][0][0] = t[num_order_ + i];
      buffer->r_[i][1][1] = t[num_order_ - i];
      buffer->r_[i][0][1] = h[num_order_ + i];
      buffer->r_[i][1][0] = h[num_order_ - i];
    }

    if (coefficients_modification_) {
      const double d0(t[num_order_]);
      for (int i(0); i < length; i += 2) {
        buffer->r_[i][0][0] += d0;
        buffer->r_[i][1][1] += d0;
      }
      for (int i((0 == num_order_ % 2) ? 0 : 1); i < length; i += 2) {
        buffer->r_[i][0][1] -= d0;
        buffer->r_[i][1][0] -= d0;
      }
    }

    // Set b
    const double* b(&(constant_vector[0]));
    for (int i(0); i < length; ++i) {
      buffer->b_[i][0] = b[i];
    }
  }

  // Step 1)
  {
    // Set X_0
    buffer->x_[0].FillDiagonal(1.0);

    // Set p_0
    if (!Invert2dMatrix(buffer->r_[0], &buffer->inv_)) {
      return false;
    }
    PutBar(0, buffer->b_, &buffer->bar_);
    buffer->p_[0] = buffer->inv_ * buffer->bar_;

    // Set V_x
    buffer->vx_ = buffer->r_[0];
  }

  // Step 2)
  for (int i(1); i < length; ++i) {
    // a) Calculate E_x
    buffer->ex_.Fill(0.0);
    for (int j(0); j < i; ++j) {
      buffer->ex_ += buffer->r_[i - j] * buffer->x_[j];
    }

    // b) Calculate \bar{e}_p
    buffer->ep_.Fill(0.0);
    for (int j(0); j < i; ++j) {
      buffer->ep_ += buffer->r_[i - j] * buffer->p_[j];
    }

    // c) Calculate B_x
    CrossTranspose(buffer->vx_, &buffer->tau_);
    if (!Invert2dMatrix(buffer->tau_, &buffer->inv_)) {
      return false;
    }
    buffer->bx_ = buffer->inv_ * buffer->ex_;

    // d) Update X
    for (int j(1); j < i; ++j) {
      CrossTranspose(buffer->prev_x_[i - j], &buffer->tau_);
      buffer->x_[j] -= buffer->tau_ * buffer->bx_;
    }
    buffer->x_[i] = -buffer->bx_;
    for (int j(1); j <= i; ++j) {
      buffer->prev_x_[j] = buffer->x_[j];
    }

    // d) Update V_x
    CrossTranspose(buffer->ex_, &buffer->tau_);
    buffer->vx_ -= buffer->tau_ * buffer->bx_;

    // e) Calculate \bar{g}
    CrossTranspose(buffer->vx_, &buffer->tau_);
    if (!Invert2dMatrix(buffer->tau_, &buffer->inv_)) {
      return false;
    }
    PutBar(i, buffer->b_, &buffer->bar_);
    buffer->g_ = buffer->inv_ * (buffer->bar_ - buffer->ep_);

    // f) Update \bar{p}
    for (int j(0); j < i; ++j) {
      CrossTranspose(buffer->x_[i - j], &buffer->tau_);
      buffer->p_[j] += buffer->tau_ * buffer->g_;
    }
    buffer->p_[i] = buffer->g_;
  }

  // Step 3)
  {
    double* a(&((*solution_vector)[0]));
    for (int i(0); i < length; ++i) {
      a[i] = buffer->p_[i][0][0];
    }
  }

  return true;
}

}  // namespace sptk
