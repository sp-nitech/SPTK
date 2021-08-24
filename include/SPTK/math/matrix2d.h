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

#ifndef SPTK_MATH_MATRIX2D_H_
#define SPTK_MATH_MATRIX2D_H_

#include <vector>  // std::vector

namespace sptk {

/**
 * 2D matrix.
 */
class Matrix2D {
 public:
  /**
   * Make 2D matrix.
   */
  Matrix2D();

  /**
   * @param[in] matrix 2D matrix.
   */
  Matrix2D(const Matrix2D& matrix);

  /**
   * @param[in] matrix 2D matrix.
   */
  Matrix2D& operator=(const Matrix2D& matrix);

  virtual ~Matrix2D() {
  }

  /**
   * @param[in] row Row.
   */
  double* operator[](int row) {
    return &data_.x_[row + row];
  }

  /**
   * @param[in] row Row.
   */
  const double* operator[](int row) const {
    return &data_.x_[row + row];
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
   * Compute sum.
   *
   * @param[in] matrix Addend.
   * @param[in,out] output Result.
   * @return True on success, false on failure.
   */
  static bool Add(const Matrix2D& matrix, Matrix2D* output);

  /**
   * Compute sum.
   *
   * @param[in] first_matrix Augend.
   * @param[in] second_matrix Addend.
   * @param[out] output Result.
   * @return True on success, false on failure.
   */
  static bool Add(const Matrix2D& first_matrix, const Matrix2D& second_matrix,
                  Matrix2D* output);

  /**
   * Compute difference.
   *
   * @param[in] matrix Subtrahend.
   * @param[in,out] output Result.
   * @return True on success, false on failure.
   */
  static bool Subtract(const Matrix2D& matrix, Matrix2D* output);

  /**
   * Compute difference.
   *
   * @param[in] first_matrix Minuend.
   * @param[in] second_matrix Subtrahend.
   * @param[out] output Result.
   * @return True on success, false on failure.
   */
  static bool Subtract(const Matrix2D& first_matrix,
                       const Matrix2D& second_matrix, Matrix2D* output);

  /**
   * Compute Ax.
   *
   * @param[in] matrix A.
   * @param[in] column_vector x.
   * @param[out] output Result.
   * @return True on success, false on failure.
   */
  static bool Multiply(const Matrix2D& matrix,
                       const std::vector<double>& column_vector,
                       std::vector<double>* output);

  /**
   * Compute AB.
   *
   * @param[in] first_matrix A.
   * @param[in] second_matrix B.
   * @param[out] output Result.
   * @return True on success, false on failure.
   */
  static bool Multiply(const Matrix2D& first_matrix,
                       const Matrix2D& second_matrix, Matrix2D* output);

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
   * Negate all elements of matrix.
   */
  void Negate(const Matrix2D& matrix);

  /**
   * Compute cross-transpose matrix.
   *
   * @param[out] transposed_matrix Cross-transposed matrix.
   * @return True on success, false on failure.
   */
  bool CrossTranspose(Matrix2D* transposed_matrix) const;

  /**
   * Compute inverse matrix.
   *
   * @param[out] inverse_matrix Inverse matrix.
   * @return True on success, false on failure.
   */
  bool Invert(Matrix2D* inverse_matrix) const;

 private:
  union {
    double x_[4];
    struct {
      double x0_;
      double x1_;
      double x2_;
      double x3_;
    } elements_;
  } data_;
};

}  // namespace sptk

#endif  // SPTK_MATH_MATRIX2D_H_
