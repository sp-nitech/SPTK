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

#ifndef SPTK_WINDOW_WINDOW_INTERFACE_H_
#define SPTK_WINDOW_WINDOW_INTERFACE_H_

#include <vector>  // std::vector

namespace sptk {

/**
 * Interface of window function.
 */
class WindowInterface {
 public:
  virtual ~WindowInterface() {
  }

  /**
   * @return Window length.
   */
  virtual int GetWindowLength() const = 0;

  /**
   * @return True if there is no problem.
   */
  virtual bool IsValid() const = 0;

  /**
   * @return Window.
   */
  virtual const std::vector<double>& Get() const = 0;
};

}  // namespace sptk

#endif  // SPTK_WINDOW_WINDOW_INTERFACE_H_
