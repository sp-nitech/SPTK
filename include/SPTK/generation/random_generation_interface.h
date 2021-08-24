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

#ifndef SPTK_GENERATION_RANDOM_GENERATION_INTERFACE_H_
#define SPTK_GENERATION_RANDOM_GENERATION_INTERFACE_H_

namespace sptk {

/**
 * An interface of random number generation.
 */
class RandomGenerationInterface {
 public:
  virtual ~RandomGenerationInterface() {
  }

  /**
   * Reset state.
   */
  virtual void Reset() = 0;

  /**
   * Get random number.
   *
   * @param[out] output Random number.
   * @return True on success, false on failure.
   */
  virtual bool Get(double* output) = 0;
};

}  // namespace sptk

#endif  // SPTK_GENERATION_RANDOM_GENERATION_INTERFACE_H_
