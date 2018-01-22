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

#include "SPTK/math/triangular_matrix.h"

#include <algorithm>  // std::fill, std::swap
#include <stdexcept>  // std::out_of_range
#include <string>     // std::string

namespace {

const std::string kErrorMessage("TriangularMatrix: Out of range");

}  // namespace

namespace sptk {

double& TriangularMatrix::Row::operator[](int col) {
  int row(row_);
  if (row < col) {
    std::swap(row, col);
  }
  return triangular_matrix_.index_[row][col];
}

const double& TriangularMatrix::Row::operator[](int col) const {
  int row(row_);
  if (row < col) {
    std::swap(row, col);
  }
  return triangular_matrix_.index_[row][col];
}

TriangularMatrix::TriangularMatrix(int num_dimension)
    : num_dimension_(num_dimension < 0 ? 0 : num_dimension) {
  data_.resize(num_dimension_ * (num_dimension_ + 1) / 2);
  index_.resize(num_dimension_);

  for (int i(0), j(0); i < num_dimension_; ++i, j += i) {
    index_[i] = &data_[j];
  }
}

TriangularMatrix::TriangularMatrix(const TriangularMatrix& triangular_matrix)
    : num_dimension_(triangular_matrix.num_dimension_) {
  data_ = triangular_matrix.data_;
  index_.resize(num_dimension_);

  for (int i(0), j(0); i < num_dimension_; ++i, j += i) {
    index_[i] = &data_[j];
  }
}

TriangularMatrix& TriangularMatrix::operator=(
    const TriangularMatrix& triangular_matrix) {
  if (this != &triangular_matrix) {
    num_dimension_ = triangular_matrix.num_dimension_;
    data_ = triangular_matrix.data_;
    index_.resize(num_dimension_);

    for (int i(0), j(0); i < num_dimension_; ++i, j += i) {
      index_[i] = &data_[j];
    }
  }
  return *this;
}

void TriangularMatrix::Resize(int num_dimension) {
  num_dimension_ = num_dimension < 0 ? 0 : num_dimension;
  data_.resize(num_dimension_ * (num_dimension_ + 1) / 2);
  index_.resize(num_dimension_);

  for (int i(0), j(0); i < num_dimension_; ++i, j += i) {
    index_[i] = &data_[j];
  }
}

double& TriangularMatrix::At(int row, int col) {
  if (row < col) {
    std::swap(row, col);
  }
  if (col < 0 || num_dimension_ <= row) {
    throw std::out_of_range(kErrorMessage);
  }
  return index_[row][col];
}

const double& TriangularMatrix::At(int row, int col) const {
  if (row < col) {
    std::swap(row, col);
  }
  if (col < 0 || num_dimension_ <= row) {
    throw std::out_of_range(kErrorMessage);
  }
  return index_[row][col];
}

void TriangularMatrix::FillZero() {
  std::fill(data_.begin(), data_.end(), 0.0);
}

}  // namespace sptk
