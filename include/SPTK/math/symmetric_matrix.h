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
//                1996-2021  Nagoya Institute of Technology          //
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

#ifndef SPTK_MATH_SYMMETRIC_MATRIX_H_
#define SPTK_MATH_SYMMETRIC_MATRIX_H_

#include <vector>  // std::vector

namespace sptk {

/**
 * Symmetric matrix.
 */
class SymmetricMatrix {
 public:
  /**
   * A row of symmetric matrix.a
   */
  class Row {
   public:
    /**
     * @param[in] matrix Symmetric matrix.
     * @param[in] row Row index.
     */
    Row(const SymmetricMatrix& matrix, int row) : matrix_(matrix), row_(row) {
    }

    virtual ~Row() {
    }

    /**
     * @param[in] column Column index.
     * @return Element.
     */
    double& operator[](int column);

    /**
     * @param[in] column Column index.
     * @return Element.
     */
    const double& operator[](int column) const;

   private:
    const SymmetricMatrix& matrix_;
    const int row_;

    friend class SymmetricMatrix;
    Row(const Row&);
    void operator=(const Row&);
  };

  /**
   * @param[in] num_dimension Size of matrix.
   */
  explicit SymmetricMatrix(int num_dimension = 0);

  /**
   * @param[in] matrix Symmetric matrix.
   */
  SymmetricMatrix(const SymmetricMatrix& matrix);

  /**
   * @param[in] matrix Symmetric matrix.
   */
  SymmetricMatrix& operator=(const SymmetricMatrix& matrix);

  virtual ~SymmetricMatrix() {
  }

  /**
   * @return Number of dimensions.
   */
  int GetNumDimension() const {
    return num_dimension_;
  }

  /**
   * Resize matrix.
   *
   * @param[in] num_dimension Number of dimensions.
   */
  void Resize(int num_dimension);

  /**
   * @param[in] row Row.
   */
  Row operator[](int row) {
    return Row(*this, row);
  }

  /**
   * @param[in] row Row.
   */
  const Row operator[](int row) const {
    return Row(*this, row);
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
   * Overwrite all elements with a value.
   *
   * @param[in] value Value.
   */
  void Fill(double value);

  /**
   * Get diagonal elements.
   *
   * @param[out] diagonal_elements Diagonal elements.
   * @return True on success, false on failure.
   */
  bool GetDiagonal(std::vector<double>* diagonal_elements) const;

  /**
   * Set diagonal elements.
   *
   * @param[in] diagonal_elements Diagonal elements.
   * @return True on success, false on failure.
   */
  bool SetDiagonal(const std::vector<double>& diagonal_elements) const;

  /**
   * Perform Cholesky decomposition.
   *
   * @param[out] lower_triangular_matrix Lower triangular matrix.
   * @param[out] diagonal_elements Diagonal elements.
   * @return True on success, false on failure.
   */
  bool CholeskyDecomposition(SymmetricMatrix* lower_triangular_matrix,
                             std::vector<double>* diagonal_elements) const;

  /**
   * Compute inverse matrix.
   *
   * @param[out] inverse_matrix Inverse matrix.
   * @return True on success, false on failure.
   */
  bool Invert(SymmetricMatrix* inverse_matrix) const;

 private:
  int num_dimension_;

  std::vector<double> data_;
  std::vector<double*> index_;
};

}  // namespace sptk

#endif  // SPTK_MATH_SYMMETRIC_MATRIX_H_
