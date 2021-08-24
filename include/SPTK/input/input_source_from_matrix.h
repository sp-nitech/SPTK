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

#ifndef SPTK_INPUT_INPUT_SOURCE_FROM_MATRIX_H_
#define SPTK_INPUT_INPUT_SOURCE_FROM_MATRIX_H_

#include <vector>  // std::vector

#include "SPTK/input/input_source_interface.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Use matrix as input source.
 */
class InputSourceFromMatrix : public InputSourceInterface {
 public:
  /**
   * @param[in] row_size Number of rows of matrix.
   * @param[in] col_size Number of columns of matrix.
   * @param[in] input_matrix Input 2D matrix.
   */
  InputSourceFromMatrix(int row_size, int col_size, double** input_matrix);

  virtual ~InputSourceFromMatrix() {
  }

  /**
   * @return Size of data.
   */
  virtual int GetSize() const {
    return row_size_;
  }

  /**
   * @return Number of rows.
   */
  int GetRowSize() const {
    return row_size_;
  }

  /**
   * @return Number of columns.
   */
  int GetColSize() const {
    return col_size_;
  }

  /**
   * @return True if this object is valid.
   */
  virtual bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[out] buffer Read data.
   * @return True on success, false on failure.
   */
  virtual bool Get(std::vector<double>* buffer);

 private:
  const int row_size_;
  const int col_size_;
  double** input_matrix_;

  int col_position_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(InputSourceFromMatrix);
};

}  // namespace sptk

#endif  // SPTK_INPUT_INPUT_SOURCE_FROM_MATRIX_H_
