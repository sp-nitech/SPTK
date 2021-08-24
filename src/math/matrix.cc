// ------------------------------------------------------------------------ //
// Copyright 2021 SPTK Working Group                                        //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ------------------------------------------------------------------------ //

#include "SPTK/math/matrix.h"

#include <algorithm>   // std::fill, std::min, std::transform
#include <functional>  // std::minus, std::negate, std::plus
#include <stdexcept>   // std::logic_error, std::out_of_range

namespace {

const char* kErrorMessageForOutOfRange("Matrix: Out of range");
const char* kErrorMessageForLogicError("Matrix: Matrix sizes do not match");

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

  Fill(0.0);
  for (int i(0); i < num_row_; ++i) {
    index_[i] = &data_[i * num_column_];
  }
}

double& Matrix::At(int row, int column) {
  if (row < 0 || num_row_ <= row || column < 0 || num_column_ <= column) {
    throw std::out_of_range(kErrorMessageForOutOfRange);
  }
  return index_[row][column];
}

const double& Matrix::At(int row, int column) const {
  if (row < 0 || num_row_ <= row || column < 0 || num_column_ <= column) {
    throw std::out_of_range(kErrorMessageForOutOfRange);
  }
  return index_[row][column];
}

Matrix& Matrix::operator+=(const Matrix& matrix) {
  if (num_row_ != matrix.num_row_ || num_column_ != matrix.num_column_) {
    throw std::logic_error(kErrorMessageForLogicError);
  }
  std::transform(data_.begin(), data_.end(), matrix.data_.begin(),
                 data_.begin(), std::plus<double>());
  return *this;
}

Matrix& Matrix::operator-=(const Matrix& matrix) {
  if (num_row_ != matrix.num_row_ || num_column_ != matrix.num_column_) {
    throw std::logic_error(kErrorMessageForLogicError);
  }
  std::transform(data_.begin(), data_.end(), matrix.data_.begin(),
                 data_.begin(), std::minus<double>());
  return *this;
}

Matrix& Matrix::operator*=(const Matrix& matrix) {
  *this = *this * matrix;
  return *this;
}

Matrix Matrix::operator+(const Matrix& matrix) const {
  Matrix result(*this);
  return result += matrix;
}

Matrix Matrix::operator-(const Matrix& matrix) const {
  Matrix result(*this);
  return result -= matrix;
}

Matrix Matrix::operator*(const Matrix& matrix) const {
  if (num_column_ != matrix.num_row_) {
    throw std::logic_error(kErrorMessageForLogicError);
  }
  Matrix result(num_row_, matrix.num_column_);
  for (int i(0); i < num_row_; ++i) {
    for (int j(0); j < matrix.num_column_; ++j) {
      for (int k(0); k < num_column_; ++k) {
        result.index_[i][j] += index_[i][k] * matrix.index_[k][j];
      }
    }
  }
  return result;
}

Matrix Matrix::operator-() const {
  Matrix result(*this);
  result.Negate();
  return result;
}

void Matrix::Fill(double value) {
  std::fill(data_.begin(), data_.end(), value);
}

void Matrix::FillDiagonal(double value) {
  Fill(0.0);
  const int num_ones(std::min(num_row_, num_column_));
  for (int i(0); i < num_ones; ++i) {
    index_[i][i] = value;
  }
}

void Matrix::Negate() {
  std::transform(data_.begin(), data_.end(), data_.begin(),
                 std::negate<double>());
}

bool Matrix::Transpose(Matrix* transposed_matrix) const {
  if (NULL == transposed_matrix || this == transposed_matrix) {
    return false;
  }

  if (transposed_matrix->num_row_ != num_column_ ||
      transposed_matrix->num_column_ != num_row_) {
    transposed_matrix->Resize(num_column_, num_row_);
  }

  for (int i(0); i < num_column_; ++i) {
    for (int j(0); j < num_row_; ++j) {
      transposed_matrix->index_[i][j] = index_[j][i];
    }
  }

  return true;
}

bool Matrix::GetSubmatrix(int row_offset, int num_row_of_submatrix,
                          int column_offset, int num_column_of_submatrix,
                          Matrix* submatrix) const {
  if (row_offset < 0 || num_row_of_submatrix <= 0 ||
      num_row_ < row_offset + num_row_of_submatrix || column_offset < 0 ||
      num_column_of_submatrix <= 0 ||
      num_column_ < column_offset + num_column_of_submatrix ||
      NULL == submatrix || this == submatrix) {
    return false;
  }

  if (submatrix->num_row_ != num_row_of_submatrix ||
      submatrix->num_column_ != num_column_of_submatrix) {
    submatrix->Resize(num_row_of_submatrix, num_column_of_submatrix);
  }

  for (int i(0); i < num_row_of_submatrix; ++i) {
    for (int j(0); j < num_column_of_submatrix; ++j) {
      submatrix->index_[i][j] = index_[row_offset + i][column_offset + j];
    }
  }

  return true;
}

bool Matrix::GetDeterminant(double* determinant) const {
  if (num_row_ != num_column_ || num_row_ <= 0 || NULL == determinant) {
    return false;
  }

  const int num_dimension(num_row_);
  if (1 == num_dimension) {
    *determinant = index_[0][0];
    return true;
  }

  const int num_order(num_dimension - 1);
  Matrix submatrix(num_order, num_order);
  *determinant = 0.0;
  for (int i(0); i < num_dimension; ++i) {
    for (int row(0), offset(0); row < num_order; ++row) {
      if (i == row) {
        offset = 1;
      }
      for (int column(0); column < num_order; ++column) {
        submatrix[row][column] = index_[row + offset][column + 1];
      }
    }
    double determinant_of_submatrix;
    if (!submatrix.GetDeterminant(&determinant_of_submatrix)) {
      return false;
    }
    *determinant += ((0 == i % 2) ? index_[i][0] : -index_[i][0]) *
                    determinant_of_submatrix;
  }
  return true;
}

}  // namespace sptk
