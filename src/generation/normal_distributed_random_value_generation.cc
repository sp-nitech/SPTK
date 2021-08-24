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

#include "SPTK/generation/normal_distributed_random_value_generation.h"

#include <cmath>  // std::log, std::sqrt

namespace {

// Generate pseudorandom number.
double PseudoRandomGeneration(std::uint64_t* next) {
  if (NULL == next) {
    return 0.0;
  }

  *next = (*next) * 1103515245L + 12345;
  const double r(static_cast<double>(((*next) / 65536L) % 32768L));

  return r / 32767.0;
}

}  // namespace

namespace sptk {

NormalDistributedRandomValueGeneration::NormalDistributedRandomValueGeneration(
    int seed)
    : seed_(seed), switch_(true) {
  next_ = static_cast<std::uint64_t>(seed_);
}

void NormalDistributedRandomValueGeneration::Reset() {
  next_ = static_cast<std::uint64_t>(seed_);
  switch_ = true;
}

bool NormalDistributedRandomValueGeneration::Get(double* output) {
  if (NULL == output) {
    return false;
  }

  if (switch_) {
    switch_ = false;
    do {
      r1_ = 2.0 * PseudoRandomGeneration(&(next_)) - 1.0;
      r2_ = 2.0 * PseudoRandomGeneration(&(next_)) - 1.0;
      s_ = r1_ * r1_ + r2_ * r2_;
    } while (0.0 == s_ || 1.0 <= s_);
    s_ = std::sqrt(-2.0 * std::log(s_) / s_);
    *output = r1_ * s_;
  } else {
    switch_ = true;
    *output = r2_ * s_;
  }

  return true;
}

}  // namespace sptk
