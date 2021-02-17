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

#include "SPTK/compression/inverse_uniform_quantization.h"

#include <cmath>  // std::pow

namespace sptk {

InverseUniformQuantization::InverseUniformQuantization(
    double absolute_maximum_value, int num_bit,
    UniformQuantization::QuantizationType quantization_type)
    : absolute_maximum_value_(absolute_maximum_value),
      num_bit_(num_bit),
      quantization_type_(quantization_type),
      is_valid_(true) {
  if (absolute_maximum_value_ <= 0.0 || num_bit_ <= 0) {
    is_valid_ = false;
    return;
  }

  switch (quantization_type_) {
    case UniformQuantization::QuantizationType::kMidRise: {
      quantization_levels_ = static_cast<int>(std::pow(2.0, num_bit_));
      break;
    }
    case UniformQuantization::QuantizationType::kMidTread: {
      quantization_levels_ = static_cast<int>(std::pow(2.0, num_bit_)) - 1;
      break;
    }
    default: {
      is_valid_ = false;
      return;
    }
  }

  step_size_ = (2.0 * absolute_maximum_value_) / quantization_levels_;
}

bool InverseUniformQuantization::Run(int input, double* output) const {
  // Check inputs.
  if (!is_valid_ || NULL == output) {
    return false;
  }

  double value;
  switch (quantization_type_) {
    case UniformQuantization::QuantizationType::kMidRise: {
      value = (input - quantization_levels_ / 2 + 0.5) * step_size_;
      break;
    }
    case UniformQuantization::QuantizationType::kMidTread: {
      value = (input - (quantization_levels_ - 1) / 2) * step_size_;
      break;
    }
    default: { return false; }
  }

  // Clip value.
  if (value < -absolute_maximum_value_) {
    value = -absolute_maximum_value_;
  } else if (absolute_maximum_value_ < value) {
    value = absolute_maximum_value_;
  }

  *output = value;

  return true;
}

}  // namespace sptk
