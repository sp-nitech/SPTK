// ----------------------------------------------------------------- //
//             The Speech Signal Processing Toolkit (SPTK)           //
//             developed by SPTK Working Group                       //
//             http://sp-tk.sourceforge.net/                         //
// ----------------------------------------------------------------- //
//                                                                   //
//  Copyright (c) 1984-2007  Tokyo Institute of Technology           //
//                           Interdisciplinary Graduate School of    //
//                           Science and Engineering                 //
//                                                                   //
//                1996-2020  Nagoya Institute of Technology          //
//                           Department of Computer Science          //
//                                                                   //
// All rights reserved.                                              //
//                                                                   //
// Redistribution and use in source and binary forms, with or        //
// without modification, are permitted provided that the following   //
// conditions are met:                                               //
//                                                                   //
// - Redistributions of source code must retain the above copyright  //
//   notice, this list of conditions and the following disclaimer.   //
// - Redistributions in binary form must reproduce the above         //
//   copyright notice, this list of conditions and the following     //
//   disclaimer in the documentation and/or other materials provided //
//   with the distribution.                                          //
// - Neither the name of the SPTK working group nor the names of its //
//   contributors may be used to endorse or promote products derived //
//   from this software without specific prior written permission.   //
//                                                                   //
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            //
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       //
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          //
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          //
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS //
// BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          //
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   //
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     //
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON //
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   //
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    //
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           //
// POSSIBILITY OF SUCH DAMAGE.                                       //
// ----------------------------------------------------------------- //

#include "SPTK/generation/normal_distributed_random_value_generation.h"

#include <cmath>  // std::log, std::sqrt

namespace {

// Generate pseudorandom number.
double PseudoRandomGeneration(std::uint64_t* next) {
  if (NULL == next) {
    return 0.0;
  }

  *next = (*next) * 1103515245L + 12345;
  const double r(((*next) / 65536L) % 32768L);

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
