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

#ifndef SPTK_INPUT_INPUT_SOURCE_FILLING_MAGIC_NUMBER_H_
#define SPTK_INPUT_INPUT_SOURCE_FILLING_MAGIC_NUMBER_H_

#include <deque>   // std::deque
#include <vector>  // std::vector

#include "SPTK/input/input_source_interface.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Interpolate magic numbers in input source.
 */
class InputSourceFillingMagicNumber : public InputSourceInterface {
 public:
  /**
   * @param[in] magic_number Magic number.
   * @param[in] source Input source.
   */
  InputSourceFillingMagicNumber(double magic_number,
                                InputSourceInterface* source);

  virtual ~InputSourceFillingMagicNumber() {
  }

  /**
   * @return Magic number.
   */
  double GetMagicNumber() const {
    return magic_number_;
  }

  /**
   * @return Size of data.
   */
  virtual int GetSize() const {
    return source_ ? source_->GetSize() : 0;
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
  int Proceed();

  const double magic_number_;

  InputSourceInterface* source_;
  bool is_valid_;

  std::deque<std::vector<double> > queue_;
  std::vector<double> final_output_;
  std::vector<int> magic_number_region_;

  DISALLOW_COPY_AND_ASSIGN(InputSourceFillingMagicNumber);
};

}  // namespace sptk

#endif  // SPTK_INPUT_INPUT_SOURCE_FILLING_MAGIC_NUMBER_H_
