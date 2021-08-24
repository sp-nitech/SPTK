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

#ifndef SPTK_INPUT_INPUT_SOURCE_INTERFACE_H_
#define SPTK_INPUT_INPUT_SOURCE_INTERFACE_H_

#include <vector>  // std::vector

namespace sptk {

/**
 * Input source interface.
 */
class InputSourceInterface {
 public:
  virtual ~InputSourceInterface() {
  }

  /**
   * @return Size of data.
   */
  virtual int GetSize() const = 0;

  /**
   * @return True if this object is valid.
   */
  virtual bool IsValid() const = 0;

  /**
   * @param[out] buffer Read data.
   * @return True on success, false on failure.
   */
  virtual bool Get(std::vector<double>* buffer) = 0;
};

}  // namespace sptk

#endif  // SPTK_INPUT_INPUT_SOURCE_INTERFACE_H_
