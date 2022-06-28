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

#ifndef SPTK_INPUT_INPUT_SOURCE_DELAY_H_
#define SPTK_INPUT_INPUT_SOURCE_DELAY_H_

#include <queue>   // std::queue
#include <vector>  // std::vector

#include "SPTK/input/input_source_interface.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Delay input source.
 */
class InputSourceDelay : public InputSourceInterface {
 public:
  /**
   * @param[in] delay Amount of delay. Negative value is allowed.
   * @param[in] keep_sequence_length If true, keep original sequence length.
   * @param[in] source Input source.
   */
  InputSourceDelay(int delay, bool keep_sequence_length,
                   InputSourceInterface* source);

  virtual ~InputSourceDelay() {
  }

  /**
   * @return Delay.
   */
  int GetDelay() const {
    return delay_;
  }

  /**
   * @return Keep sequence length flag.
   */
  bool KeepSequenceLength() const {
    return keep_sequence_length_;
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
  const int delay_;
  const bool keep_sequence_length_;

  InputSourceInterface* source_;
  bool is_valid_;

  std::queue<std::vector<double> > queue_;
  int num_zeros_;

  DISALLOW_COPY_AND_ASSIGN(InputSourceDelay);
};

}  // namespace sptk

#endif  // SPTK_INPUT_INPUT_SOURCE_DELAY_H_
