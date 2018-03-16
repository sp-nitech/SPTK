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

#include "SPTK/math/matrix.h"

#include <algorithm>  // std::fill
#include <stdexcept>  // std::out_of_range
#include <string>     // std::string

namespace {

const std::string kErrorMessage("Matrix: Out of range");

}  // namespace

namespace sptk {

Matrix::Matrix(int num_row, int num_column)
    : num_row_(num_row < 0 ? 0 : num_row),
      num_column_(num_column < 0 ? 0 : num_column) {
  data_.resize(num_row_ * num_column_);
  index_.resize(num_row_);

  for (int i(0); i < num_row_; ++i) {
    index_[i] = &data_[i * num_column_];
  }
}

Matrix::Matrix(int num_row, int num_column, const std::vector<double>& vector)
    : num_row_(num_row < 0 ? 0 : num_row),
      num_column_(num_column < 0 ? 0 : num_column) {
  data_ = vector;
  index_.resize(num_row_);

  for (int i(0); i < num_row_; ++i) {
    index_[i] = &data_[i * num_column_];
  }
}

Matrix::Matrix(const Matrix& matrix)
    : num_row_(matrix.num_row_), num_column_(matrix.num_column_) {
  data_ = matrix.data_;
  index_.resize(num_row_);

  for (int i(0); i < num_row_; ++i) {
    index_[i] = &data_[i * num_column_];
  }
}

Matrix& Matrix::operator=(const Matrix& matrix) {
  if (this != &matrix) {
    num_row_ = matrix.num_row_;
    num_column_ = matrix.num_column_;
    data_ = matrix.data_;
    index_.resize(num_row_);

    for (int i(0); i < num_row_; ++i) {
      index_[i] = &data_[i * num_column_];
    }
  }
  return *this;
}

void Matrix::Resize(int num_row, int num_column) {
  num_row_ = num_row < 0 ? 0 : num_row;
  num_column_ = num_column < 0 ? 0 : num_column;
  data_.resize(num_row_ * num_column_);
  index_.resize(num_row_);

  FillZero();
  for (int i(0); i < num_row_; ++i) {
    index_[i] = &data_[i * num_column_];
  }
}

double& Matrix::At(int row, int column) {
  if (row < 0 || num_row_ <= row || column < 0 || num_column_ <= column) {
    throw std::out_of_range(kErrorMessage);
  }
  return index_[row][column];
}

const double& Matrix::At(int row, int column) const {
  if (row < 0 || num_row_ <= row || column < 0 || num_column_ <= column) {
    throw std::out_of_range(kErrorMessage);
  }
  return index_[row][column];
}

void Matrix::FillZero() {
  std::fill(data_.begin(), data_.end(), 0.0);
}

bool Matrix::Transpose(Matrix* transposed_matrix) const {
  if (NULL == transposed_matrix) {
    return false;
  }

  if (num_column_ != transposed_matrix->num_row_ ||
      num_row_ != transposed_matrix->num_column_) {
    transposed_matrix->Resize(num_column_, num_row_);
  }

  for (int i(0); i < num_row_; ++i) {
    for (int j(0); j < num_column_; ++j) {
      transposed_matrix->index_[j][i] = index_[i][j];
    }
  }

  return true;
}

}  // namespace sptk
