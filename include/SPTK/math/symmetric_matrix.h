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
