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

#include "SPTK/math/matrix2d.h"

#include <cmath>      // std::fabs
#include <stdexcept>  // std::out_of_range

namespace {

const char* kErrorMessageForOutOfRange("Matrix2D: Out of range");
const double kMinimumValueOfDeterminant(1e-6);

}  // namespace

namespace sptk {

Matrix2D::Matrix2D() {
  data_.elements_.x0_ = 0.0;
  data_.elements_.x1_ = 0.0;
  data_.elements_.x2_ = 0.0;
  data_.elements_.x3_ = 0.0;
}

Matrix2D::Matrix2D(const Matrix2D& matrix) {
  data_.elements_.x0_ = matrix.data_.elements_.x0_;
  data_.elements_.x1_ = matrix.data_.elements_.x1_;
  data_.elements_.x2_ = matrix.data_.elements_.x2_;
  data_.elements_.x3_ = matrix.data_.elements_.x3_;
}

Matrix2D& Matrix2D::operator=(const Matrix2D& matrix) {
  if (this != &matrix) {
    data_.elements_.x0_ = matrix.data_.elements_.x0_;
    data_.elements_.x1_ = matrix.data_.elements_.x1_;
    data_.elements_.x2_ = matrix.data_.elements_.x2_;
    data_.elements_.x3_ = matrix.data_.elements_.x3_;
  }
  return *this;
}

double& Matrix2D::At(int row, int column) {
  if (row < 0 || 2 <= row || column < 0 || 2 <= column) {
    throw std::out_of_range(kErrorMessageForOutOfRange);
  }
  return data_.x_[row + row + column];
}

const double& Matrix2D::At(int row, int column) const {
  if (row < 0 || 2 <= row || column < 0 || 2 <= column) {
    throw std::out_of_range(kErrorMessageForOutOfRange);
  }
  return data_.x_[row + row + column];
}

bool Matrix2D::Add(const Matrix2D& matrix, Matrix2D* output) {
  if (NULL == output) {
    return false;
  }
  output->data_.elements_.x0_ += matrix.data_.elements_.x0_;
  output->data_.elements_.x1_ += matrix.data_.elements_.x1_;
  output->data_.elements_.x2_ += matrix.data_.elements_.x2_;
  output->data_.elements_.x3_ += matrix.data_.elements_.x3_;
  return true;
}

bool Matrix2D::Add(const Matrix2D& first_matrix, const Matrix2D& second_matrix,
                   Matrix2D* output) {
  if (NULL == output) {
    return false;
  }
  output->data_.elements_.x0_ =
      first_matrix.data_.elements_.x0_ + second_matrix.data_.elements_.x0_;
  output->data_.elements_.x1_ =
      first_matrix.data_.elements_.x1_ + second_matrix.data_.elements_.x1_;
  output->data_.elements_.x2_ =
      first_matrix.data_.elements_.x2_ + second_matrix.data_.elements_.x2_;
  output->data_.elements_.x3_ =
      first_matrix.data_.elements_.x3_ + second_matrix.data_.elements_.x3_;
  return true;
}

bool Matrix2D::Subtract(const Matrix2D& matrix, Matrix2D* output) {
  if (NULL == output) {
    return false;
  }
  output->data_.elements_.x0_ -= matrix.data_.elements_.x0_;
  output->data_.elements_.x1_ -= matrix.data_.elements_.x1_;
  output->data_.elements_.x2_ -= matrix.data_.elements_.x2_;
  output->data_.elements_.x3_ -= matrix.data_.elements_.x3_;
  return true;
}

bool Matrix2D::Subtract(const Matrix2D& first_matrix,
                        const Matrix2D& second_matrix, Matrix2D* output) {
  if (NULL == output) {
    return false;
  }
  output->data_.elements_.x0_ =
      first_matrix.data_.elements_.x0_ - second_matrix.data_.elements_.x0_;
  output->data_.elements_.x1_ =
      first_matrix.data_.elements_.x1_ - second_matrix.data_.elements_.x1_;
  output->data_.elements_.x2_ =
      first_matrix.data_.elements_.x2_ - second_matrix.data_.elements_.x2_;
  output->data_.elements_.x3_ =
      first_matrix.data_.elements_.x3_ - second_matrix.data_.elements_.x3_;
  return true;
}

bool Matrix2D::Multiply(const Matrix2D& matrix,
                        const std::vector<double>& column_vector,
                        std::vector<double>* output) {
  if (NULL == output || output == &column_vector) {
    return false;
  }
  (*output)[0] = matrix.data_.elements_.x0_ * column_vector[0] +
                 matrix.data_.elements_.x1_ * column_vector[1];
  (*output)[1] = matrix.data_.elements_.x2_ * column_vector[0] +
                 matrix.data_.elements_.x3_ * column_vector[1];
  return true;
}

bool Matrix2D::Multiply(const Matrix2D& first_matrix,
                        const Matrix2D& second_matrix, Matrix2D* output) {
  if (NULL == output || output == &first_matrix || output == &second_matrix) {
    return false;
  }
  output->data_.elements_.x0_ =
      first_matrix.data_.elements_.x0_ * second_matrix.data_.elements_.x0_ +
      first_matrix.data_.elements_.x1_ * second_matrix.data_.elements_.x2_;
  output->data_.elements_.x1_ =
      first_matrix.data_.elements_.x0_ * second_matrix.data_.elements_.x1_ +
      first_matrix.data_.elements_.x1_ * second_matrix.data_.elements_.x3_;
  output->data_.elements_.x2_ =
      first_matrix.data_.elements_.x2_ * second_matrix.data_.elements_.x0_ +
      first_matrix.data_.elements_.x3_ * second_matrix.data_.elements_.x2_;
  output->data_.elements_.x3_ =
      first_matrix.data_.elements_.x2_ * second_matrix.data_.elements_.x1_ +
      first_matrix.data_.elements_.x3_ * second_matrix.data_.elements_.x3_;
  return true;
}

void Matrix2D::Fill(double value) {
  data_.elements_.x0_ = value;
  data_.elements_.x1_ = value;
  data_.elements_.x2_ = value;
  data_.elements_.x3_ = value;
}

void Matrix2D::FillDiagonal(double value) {
  data_.elements_.x0_ = value;
  data_.elements_.x1_ = 0.0;
  data_.elements_.x2_ = 0.0;
  data_.elements_.x3_ = value;
}

void Matrix2D::Negate() {
  data_.elements_.x0_ = -data_.elements_.x0_;
  data_.elements_.x1_ = -data_.elements_.x1_;
  data_.elements_.x2_ = -data_.elements_.x2_;
  data_.elements_.x3_ = -data_.elements_.x3_;
}

void Matrix2D::Negate(const Matrix2D& matrix) {
  data_.elements_.x0_ = -matrix.data_.elements_.x0_;
  data_.elements_.x1_ = -matrix.data_.elements_.x1_;
  data_.elements_.x2_ = -matrix.data_.elements_.x2_;
  data_.elements_.x3_ = -matrix.data_.elements_.x3_;
}

bool Matrix2D::CrossTranspose(Matrix2D* transposed_matrix) const {
  if (NULL == transposed_matrix || this == transposed_matrix) {
    return false;
  }
  transposed_matrix->data_.elements_.x0_ = data_.elements_.x3_;
  transposed_matrix->data_.elements_.x1_ = data_.elements_.x2_;
  transposed_matrix->data_.elements_.x2_ = data_.elements_.x1_;
  transposed_matrix->data_.elements_.x3_ = data_.elements_.x0_;
  return true;
}

bool Matrix2D::Invert(Matrix2D* inverse_matrix) const {
  const double determinant(data_.elements_.x0_ * data_.elements_.x3_ -
                           data_.elements_.x1_ * data_.elements_.x2_);
  if (NULL == inverse_matrix || this == inverse_matrix ||
      std::fabs(determinant) < kMinimumValueOfDeterminant) {
    return false;
  }

  const double inverse_of_determinant(1.0 / determinant);
  inverse_matrix->data_.elements_.x0_ =
      data_.elements_.x3_ * inverse_of_determinant;
  inverse_matrix->data_.elements_.x1_ =
      -data_.elements_.x1_ * inverse_of_determinant;
  inverse_matrix->data_.elements_.x2_ =
      -data_.elements_.x2_ * inverse_of_determinant;
  inverse_matrix->data_.elements_.x3_ =
      data_.elements_.x0_ * inverse_of_determinant;
  return true;
}

}  // namespace sptk
