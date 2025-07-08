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

#ifndef SPTK_GENERATION_UNIFORM_DISTRIBUTED_RANDOM_VALUE_GENERATION_H_
#define SPTK_GENERATION_UNIFORM_DISTRIBUTED_RANDOM_VALUE_GENERATION_H_

#include <cstdint>  // std::uint64_t

#include "SPTK/generation/random_generation_interface.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Generate random number based on uniform distribution.
 */
class UniformDistributedRandomValueGeneration
    : public RandomGenerationInterface {
 public:
  /**
   * @param[in] seed Random seed.
   * @param[in] lower_bound Lower bound of random number.
   * @param[in] upper_bound Upper bound of random number.
   */
  explicit UniformDistributedRandomValueGeneration(int seed,
                                                   double lower_bound = 0.0,
                                                   double upper_bound = 1.0);

  ~UniformDistributedRandomValueGeneration() override {
  }

  /**
   * Reset internal state.
   */
  void Reset() override;

  /**
   * Get random number.
   *
   * @param[out] output Random number.
   * @return True on success, false on failure.
   */
  bool Get(double* output) override;

  /**
   * @return Random seed.
   */
  int GetSeed() const {
    return seed_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

 private:
  const std::uint64_t seed_;
  const double lower_bound_;
  const double upper_bound_;

  std::uint64_t next_;
  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(UniformDistributedRandomValueGeneration);
};

}  // namespace sptk

#endif  // SPTK_GENERATION_UNIFORM_DISTRIBUTED_RANDOM_VALUE_GENERATION_H_
