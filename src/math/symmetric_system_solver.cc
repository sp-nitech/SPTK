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

#include "SPTK/math/symmetric_system_solver.h"

#include <cstddef>  // std::size_t

namespace sptk {

SymmetricSystemSolver::SymmetricSystemSolver(int num_order)
    : num_order_(num_order), is_valid_(true) {
  if (num_order_ < 0) {
    is_valid_ = false;
  }
}

bool SymmetricSystemSolver::Run(const SymmetricMatrix& coefficient_matrix,
                                const std::vector<double>& constant_vector,
                                std::vector<double>* solution_vector,
                                SymmetricSystemSolver::Buffer* buffer) const {
  // Check inputs.
  const int length(num_order_ + 1);
  if (!is_valid_ || coefficient_matrix.GetNumDimension() != length ||
      constant_vector.size() != static_cast<std::size_t>(length) ||
      NULL == solution_vector || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  if (solution_vector->size() != static_cast<std::size_t>(length)) {
    solution_vector->resize(length);
  }
  if (buffer->inverse_matrix_.GetNumDimension() != length) {
    buffer->inverse_matrix_.Resize(length);
  }

  const double* b(&constant_vector[0]);
  double* x(&((*solution_vector)[0]));

  SymmetricMatrix lower_triangular_matrix(length);
  std::vector<double> diagonal_elements(length);
  if (!coefficient_matrix.CholeskyDecomposition(&lower_triangular_matrix,
                                                &diagonal_elements)) {
    return false;
  }

  std::vector<double> y(length);
  for (int i(0); i < length; ++i) {
    y[i] = b[i];
    for (int j(0); j < i; ++j) {
      y[i] -= lower_triangular_matrix[i][j] * y[j];
    }
  }

  for (int i(length - 1); 0 <= i; --i) {
    x[i] = y[i] / diagonal_elements[i];
    for (int j(i + 1); j < length; ++j) {
      x[i] -= lower_triangular_matrix[j][i] * x[j];
    }
  }

  return true;
}

}  // namespace sptk
