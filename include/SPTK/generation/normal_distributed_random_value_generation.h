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

#ifndef SPTK_GENERATION_NORMAL_DISTRIBUTED_RANDOM_VALUE_GENERATION_H_
#define SPTK_GENERATION_NORMAL_DISTRIBUTED_RANDOM_VALUE_GENERATION_H_

#include <cstdint>  // std::uint64_t

#include "SPTK/generation/random_generation_interface.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Generate random number based on normal distribution.
 */
class NormalDistributedRandomValueGeneration
    : public RandomGenerationInterface {
 public:
  /**
   * @param[in] seed Random seed.
   */
  explicit NormalDistributedRandomValueGeneration(int seed);

  virtual ~NormalDistributedRandomValueGeneration() {
  }

  /**
   * Reset internal state.
   */
  virtual void Reset();

  /**
   * Get random number.
   *
   * @param[out] output Random number.
   * @return True on success, false on failure.
   */
  virtual bool Get(double* output);

  /**
   * @return Random seed.
   */
  int GetSeed() const {
    return seed_;
  }

 private:
  const int seed_;

  std::uint64_t next_;
  bool switch_;
  double r1_;
  double r2_;
  double s_;

  DISALLOW_COPY_AND_ASSIGN(NormalDistributedRandomValueGeneration);
};

}  // namespace sptk

#endif  // SPTK_GENERATION_NORMAL_DISTRIBUTED_RANDOM_VALUE_GENERATION_H_
