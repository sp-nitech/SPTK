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

#ifndef SPTK_MATH_MATRIX_H_
#define SPTK_MATH_MATRIX_H_

#include <vector>  // std::vector

namespace sptk {

/**
 * Matrix.
 */
class Matrix {
 public:
  /**
   * @param[in] num_row Number of rows.
   * @param[in] num_column Number of columns.
   */
  explicit Matrix(int num_row = 0, int num_column = 0);

  /**
   * @param[in] num_row Number of rows.
   * @param[in] num_column Number of columns.
   * @param[in] vector Diagonal elements.
   */
  Matrix(int num_row, int num_column, const std::vector<double>& vector);

  /**
   * @param[in] matrix Matrix.
   */
  Matrix(const Matrix& matrix);

  /**
   * @param[in] matrix Matrix.
   */
  Matrix& operator=(const Matrix& matrix);

  virtual ~Matrix() {
  }

  /**
   * @return Number of rows.
   */
  int GetNumRow() const {
    return num_row_;
  }

  /**
   * @return Number of columns.
   */
  int GetNumColumn() const {
    return num_column_;
  }

  /**
   * Resize matrix.
   *
   * @param[in] num_row Number of rows.
   * @param[in] num_column Number of columns.
   */
  void Resize(int num_row, int num_column);

  /**
   * @param[in] row Row.
   */
  double* operator[](int row) {
    return index_[row];
  }

  /**
   * @param[in] row Row.
   */
  const double* operator[](int row) const {
    return index_[row];
  }

  /**
   * Get element.
   *
   * @param[in] row i.
   * @param[in] column j.
   * @return (i, j)-th element.
   */
  double& At(int row, int column);

  /**
   * Get element.
   *
   * @param[in] row i.
   * @param[in] column j.
   * @return (i, j)-th element.
   */
  const double& At(int row, int column) const;

  /**
   * @param[in] matrix Addend.
   * @return Sum.
   */
  Matrix& operator+=(const Matrix& matrix);

  /**
   * @param[in] matrix Subtrahend.
   * @return Difference.
   */
  Matrix& operator-=(const Matrix& matrix);

  /**
   * @param[in] matrix Multiplicand.
   * @return Product.
   */
  Matrix& operator*=(const Matrix& matrix);

  /**
   * @param[in] matrix Addend.
   * @return Sum.
   */
  Matrix operator+(const Matrix& matrix) const;

  /**
   * @param[in] matrix Subtrahend.
   * @return Difference.
   */
  Matrix operator-(const Matrix& matrix) const;

  /**
   * @param[in] matrix Multiplicand.
   * @return Product.
   */
  Matrix operator*(const Matrix& matrix) const;

  /**
   * Negate.
   *
   * @return Negated matrix.
   */
  Matrix operator-() const;

  /**
   * Overwrite all elements with a value.
   *
   * @param[in] value Value.
   */
  void Fill(double value);

  /**
   * Overwrite diagonal elements with a value.
   *
   * @param[in] value Diagonal value.
   */
  void FillDiagonal(double value);

  /**
   * Negate all elements of matrix.
   */
  void Negate();

  /**
   * Transpose matrix.
   *
   * @param[out] transposed_matrix Transposed matrix.
   * @return True on success, false on failure.
   */
  bool Transpose(Matrix* transposed_matrix) const;

  /**
   * Get submatrix.
   *
   * @param[in] row_offset Offset of row.
   * @param[in] num_row_of_submatrix Number of rows of submatrix.
   * @param[in] column_offset Offset of column.
   * @param[in] num_column_of_submatrix Number of columns of submatrix.
   * @param[out] submatrix Submatrix.
   * @return True on success, false on failure.
   */
  bool GetSubmatrix(int row_offset, int num_row_of_submatrix, int column_offset,
                    int num_column_of_submatrix, Matrix* submatrix) const;

  /**
   * Compute determinant.
   *
   * @param[out] determinant Determinant.
   * @return True on success, false on failure.
   */
  bool GetDeterminant(double* determinant) const;

 private:
  int num_row_;
  int num_column_;

  std::vector<double> data_;
  std::vector<double*> index_;
};

}  // namespace sptk

#endif  // SPTK_MATH_MATRIX_H_
