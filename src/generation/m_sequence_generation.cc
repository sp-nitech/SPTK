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

#include "SPTK/generation/m_sequence_generation.h"

namespace {

const int kInitialValue(0x55555555);
const int kB0(0x00000001);
const int kB28(0x10000000);
const int kB31(0x80000000);
const int kB31F(0x7fffffff);

}  // namespace

namespace sptk {

MSequenceGeneration::MSequenceGeneration() : x_(kInitialValue) {
}

void MSequenceGeneration::Reset() {
  x_ = kInitialValue;
}

bool MSequenceGeneration::Get(double* output) {
  if (NULL == output) {
    return false;
  }

  // Generate m-sequence using X**31 + X**28 + 1.
  int x0, x28;

  x_ >>= 1;

  if (x_ & kB0) {
    x0 = 1;
  } else {
    x0 = -1;
  }

  if (x_ & kB28) {
    x28 = 1;
  } else {
    x28 = -1;
  }

  if (x0 + x28) {
    x_ &= kB31F;
  } else {
    x_ |= kB31;
  }

  *output = static_cast<double>(x0);

  return true;
}

}  // namespace sptk
