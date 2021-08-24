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

#include "SPTK/input/input_source_from_matrix.h"

#include <algorithm>  // std::copy
#include <cstddef>    // std::size_t

namespace sptk {

InputSourceFromMatrix::InputSourceFromMatrix(int row_size, int col_size,
                                             double** input_matrix)
    : row_size_(row_size),
      col_size_(col_size),
      input_matrix_(input_matrix),
      col_position_(0),
      is_valid_(true) {
  if (row_size_ <= 0 || col_size_ <= 0 || NULL == input_matrix_) {
    is_valid_ = false;
    return;
  }
}

bool InputSourceFromMatrix::Get(std::vector<double>* buffer) {
  if (NULL == buffer || !is_valid_ || col_size_ <= col_position_) {
    return false;
  }

  if (buffer->size() != static_cast<std::size_t>(row_size_)) {
    buffer->resize(row_size_);
  }

  std::copy(&(input_matrix_[col_position_][0]),
            &(input_matrix_[col_position_][row_size_]), buffer->begin());

  ++col_position_;

  return true;
}

}  // namespace sptk
