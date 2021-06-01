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

#include "SPTK/math/symmetric_matrix.h"

#include <algorithm>  // std::fill, std::swap
#include <cstddef>    // std::size_t
#include <stdexcept>  // std::out_of_range

#include "SPTK/math/matrix.h"

namespace {

const char* kErrorMessage("SymmetricMatrix: Out of range");

}  // namespace

namespace sptk {

double& SymmetricMatrix::Row::operator[](int column) {
  int row(row_);
  if (row < column) {
    std::swap(row, column);
  }
  return symmetric_matrix_.index_[row][column];
}

const double& SymmetricMatrix::Row::operator[](int column) const {
  int row(row_);
  if (row < column) {
    std::swap(row, column);
  }
  return symmetric_matrix_.index_[row][column];
}

SymmetricMatrix::SymmetricMatrix(int num_dimension)
    : num_dimension_(num_dimension < 0 ? 0 : num_dimension) {
  data_.resize(num_dimension_ * (num_dimension_ + 1) / 2);
  index_.resize(num_dimension_);

  for (int i(0), j(0); i < num_dimension_; ++i, j += i) {
    index_[i] = &data_[j];
  }
}

SymmetricMatrix::SymmetricMatrix(const SymmetricMatrix& symmetric_matrix)
    : num_dimension_(symmetric_matrix.num_dimension_) {
  data_ = symmetric_matrix.data_;
  index_.resize(num_dimension_);

  for (int i(0), j(0); i < num_dimension_; ++i, j += i) {
    index_[i] = &data_[j];
  }
}

SymmetricMatrix& SymmetricMatrix::operator=(
    const SymmetricMatrix& symmetric_matrix) {
  if (this != &symmetric_matrix) {
    num_dimension_ = symmetric_matrix.num_dimension_;
    data_ = symmetric_matrix.data_;
    index_.resize(num_dimension_);

    for (int i(0), j(0); i < num_dimension_; ++i, j += i) {
      index_[i] = &data_[j];
    }
  }
  return *this;
}

void SymmetricMatrix::Resize(int num_dimension) {
  num_dimension_ = num_dimension < 0 ? 0 : num_dimension;
  data_.resize(num_dimension_ * (num_dimension_ + 1) / 2);
  index_.resize(num_dimension_);

  for (int i(0), j(0); i < num_dimension_; ++i, j += i) {
    index_[i] = &data_[j];
  }
}

double& SymmetricMatrix::At(int row, int column) {
  if (row < column) {
    std::swap(row, column);
  }
  if (column < 0 || num_dimension_ <= row) {
    throw std::out_of_range(kErrorMessage);
  }
  return index_[row][column];
}

const double& SymmetricMatrix::At(int row, int column) const {
  if (row < column) {
    std::swap(row, column);
  }
  if (column < 0 || num_dimension_ <= row) {
    throw std::out_of_range(kErrorMessage);
  }
  return index_[row][column];
}

void SymmetricMatrix::Fill(double value) {
  std::fill(data_.begin(), data_.end(), value);
}

bool SymmetricMatrix::GetDiagonal(
    std::vector<double>* diagonal_elements) const {
  if (NULL == diagonal_elements) {
    return false;
  }
  if (diagonal_elements->size() != static_cast<std::size_t>(num_dimension_)) {
    diagonal_elements->resize(num_dimension_);
  }
  for (int i(0); i < num_dimension_; ++i) {
    (*diagonal_elements)[i] = index_[i][i];
  }
  return true;
}

bool SymmetricMatrix::SetDiagonal(
    const std::vector<double>& diagonal_elements) const {
  if (diagonal_elements.size() != static_cast<std::size_t>(num_dimension_)) {
    return false;
  }
  for (int i(0); i < num_dimension_; ++i) {
    index_[i][i] = diagonal_elements[i];
  }
  return true;
}

bool SymmetricMatrix::CholeskyDecomposition(
    SymmetricMatrix* lower_triangular_matrix,
    std::vector<double>* diagonal_elements) const {
  if (NULL == lower_triangular_matrix || NULL == diagonal_elements ||
      this == lower_triangular_matrix || 0.0 == index_[0][0]) {
    return false;
  }

  if (lower_triangular_matrix->num_dimension_ != num_dimension_) {
    lower_triangular_matrix->Resize(num_dimension_);
  }
  if (diagonal_elements->size() != static_cast<std::size_t>(num_dimension_)) {
    diagonal_elements->resize(num_dimension_);
  }

  double* d(&((*diagonal_elements)[0]));

  d[0] = index_[0][0];
  lower_triangular_matrix->index_[0][0] = 1.0;
  for (int i(1); i < num_dimension_; ++i) {
    for (int j(0); j < i; ++j) {
      double tmp(index_[i][j]);
      for (int k(0); k < j; ++k) {
        tmp -= lower_triangular_matrix->index_[i][k] *
               lower_triangular_matrix->index_[j][k] * d[k];
      }
      lower_triangular_matrix->index_[i][j] = tmp / d[j];
    }

    d[i] = index_[i][i];
    for (int j(0); j < i; ++j) {
      d[i] -= lower_triangular_matrix->index_[i][j] *
              lower_triangular_matrix->index_[i][j] * d[j];
    }
    if (d[i] <= 0.0) {
      return false;
    }
    lower_triangular_matrix->index_[i][i] = 1.0;
  }
  return true;
}

bool SymmetricMatrix::Invert(SymmetricMatrix* inverse_matrix) const {
  if (NULL == inverse_matrix || this == inverse_matrix) {
    return false;
  }

  if (inverse_matrix->num_dimension_ != num_dimension_) {
    inverse_matrix->Resize(num_dimension_);
  }

  SymmetricMatrix lower_triangular_matrix(num_dimension_);
  std::vector<double> diagonal_elements(num_dimension_);
  if (!CholeskyDecomposition(&lower_triangular_matrix, &diagonal_elements)) {
    return false;
  }

  for (int i(num_dimension_ - 1); 0 <= i; --i) {
    inverse_matrix->index_[i][i] = 1.0;
    for (int j(i + 1); j < num_dimension_; ++j) {
      inverse_matrix->index_[j][i] =
          lower_triangular_matrix.index_[j][i] * inverse_matrix->index_[i][i];
      for (int k(i + 1); k < j; ++k) {
        inverse_matrix->index_[j][i] +=
            lower_triangular_matrix.index_[j][k] * inverse_matrix->index_[k][i];
      }
      inverse_matrix->index_[j][i] *= -inverse_matrix->index_[j][j];
    }
  }

  std::vector<double> inverse_diagonal_elements(num_dimension_);
  for (int i(0); i < num_dimension_; ++i) {
    inverse_diagonal_elements[i] = 1.0 / diagonal_elements[i];
  }

  for (int i(0); i < num_dimension_; ++i) {
    for (int j(0); j <= i; ++j) {
      double sum(0.0);
      for (int k(i); k < num_dimension_; ++k) {
        sum += inverse_matrix->index_[k][i] * inverse_diagonal_elements[k] *
               inverse_matrix->index_[k][j];
      }
      inverse_matrix->index_[i][j] = sum;
    }
  }

  return true;
}

}  // namespace sptk
