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

#ifndef SPTK_INPUT_INPUT_SOURCE_FROM_VECTOR_H_
#define SPTK_INPUT_INPUT_SOURCE_FROM_VECTOR_H_

#include <vector>  // std::vector

#include "SPTK/input/input_source_interface.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Use vector as input source.
 */
class InputSourceFromVector : public InputSourceInterface {
 public:
  /**
   * @param[in] zero_padding If true, pad with zero in the last reading.
   * @param[in] read_size Read size.
   * @param[in] input_vector Input vector.
   */
  InputSourceFromVector(bool zero_padding, int read_size,
                        std::vector<double>* input_vector);

  virtual ~InputSourceFromVector() {
  }

  /**
   * @return Size of data.
   */
  virtual int GetSize() const {
    return read_size_;
  }

  /**
   * @return Size of vector.
   */
  int VectorSize() const {
    return vector_size_;
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
  const bool zero_padding_;
  const int read_size_;
  const int vector_size_;
  const std::vector<double>* input_vector_;

  int position_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(InputSourceFromVector);
};

}  // namespace sptk

#endif  // SPTK_INPUT_INPUT_SOURCE_FROM_VECTOR_H_
