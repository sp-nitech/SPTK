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

#include <cstdint>  // std::uint64_t

#include "SPTK/generation/normal_distributed_random_value_generation.h"

namespace {

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

UniformDistributedRandomValueGeneration::
    UniformDistributedRandomValueGeneration(int seed, double lower_bound,
                                            double upper_bound)
    : seed_(static_cast<std::uint64_t>(seed)),
      lower_bound_(lower_bound),
      upper_bound_(upper_bound),
      next_(seed_),
      is_valid_(true) {
  if (upper_bound_ <= lower_bound_) {
    is_valid_ = false;
    return;
  }
}

void UniformDistributedRandomValueGeneration::Reset() {
  next_ = seed_;
}

bool UniformDistributedRandomValueGeneration::Get(double* output) {
  if (!is_valid_ || NULL == output) {
    return false;
  }
  *output = PseudoRandomGeneration(&next_);
  if (0.0 != lower_bound_ || 1.0 != upper_bound_) {
    *output = lower_bound_ + (upper_bound_ - lower_bound_) * (*output);
  }
  return true;
}

}  // namespace sptk
