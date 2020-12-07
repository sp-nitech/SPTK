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

#include "SPTK/math/entropy_calculation.h"

#include <cstddef>  // std::size_t

namespace sptk {

EntropyCalculation::EntropyCalculation(int num_element,
                                       EntropyUnits entropy_unit)
    : num_element_(num_element), entropy_unit_(entropy_unit), is_valid_(true) {
  if (num_element_ <= 0 || kNumUnits == entropy_unit_) {
    is_valid_ = false;
    return;
  }
}

bool EntropyCalculation::Run(const std::vector<double>& probability,
                             double* entropy) const {
  if (!is_valid_ ||
      probability.size() != static_cast<std::size_t>(num_element_) ||
      NULL == entropy) {
    return false;
  }

  const double* p(&(probability[0]));
  double sum(0.0);

  switch (entropy_unit_) {
    case kBit: {
      for (int i(0); i < num_element_; ++i) {
        sum += p[i] * FloorLog2(p[i]);
      }
      break;
    }
    case kNat: {
      for (int i(0); i < num_element_; ++i) {
        sum += p[i] * FloorLog(p[i]);
      }
      break;
    }
    case kDit: {
      for (int i(0); i < num_element_; ++i) {
        sum += p[i] * FloorLog10(p[i]);
      }
      break;
    }
    default: { return false; }
  }

  *entropy = -sum;

  return true;
}

}  // namespace sptk
