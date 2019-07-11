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

#ifndef SPTK_MATH_MATRIX2D_H_
#define SPTK_MATH_MATRIX2D_H_

#include <vector>  // std::vector

namespace sptk {

class Matrix2D {
 public:
  //
  Matrix2D();

  //
  Matrix2D(const Matrix2D& matrix);

  //
  Matrix2D& operator=(const Matrix2D& matrix);

  //
  virtual ~Matrix2D() {
  }

  //
  double* operator[](int row) {
    return &data_.x_[row + row];
  }

  //
  const double* operator[](int row) const {
    return &data_.x_[row + row];
  }

  //
  double& At(int row, int column);

  //
  const double& At(int row, int column) const;

  //
  static bool Add(const Matrix2D& matrix, Matrix2D* output);

  //
  static bool Add(const Matrix2D& first_matrix, const Matrix2D& second_matrix,
                  Matrix2D* output);

  //
  static bool Subtract(const Matrix2D& matrix, Matrix2D* output);

  //
  static bool Subtract(const Matrix2D& first_matrix,
                       const Matrix2D& second_matrix, Matrix2D* output);

  //
  static bool Multiply(const Matrix2D& matrix,
                       const std::vector<double>& column_vector,
                       std::vector<double>* output);

  //
  static bool Multiply(const Matrix2D& first_matrix,
                       const Matrix2D& second_matrix, Matrix2D* output);

  //
  void Fill(double value);

  //
  void FillDiagonal(double value);

  //
  void Negate();

  //
  void Negate(const Matrix2D& matrix);

  //
  bool CrossTranspose(Matrix2D* transposed_matrix) const;

  //
  bool Invert(Matrix2D* inverse_matrix) const;

 private:
  //
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
