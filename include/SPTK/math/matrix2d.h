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
