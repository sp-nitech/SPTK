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
