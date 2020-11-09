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

#include "SPTK/window/data_windowing.h"

#include <algorithm>  // std::fill, std::transform
#include <cmath>      // std::sqrt
#include <cstddef>    // std::size_t
#include <numeric>    // std::accumulate, std::inner_product

namespace sptk {

DataWindowing::DataWindowing(WindowInterface* window_interface,
                             int output_length,
                             NormalizationType normalization_type)
    : input_length_(window_interface ? window_interface->GetWindowLength() : 0),
      output_length_(output_length),
      is_valid_(true) {
  if (input_length_ <= 0 || output_length_ < input_length_) {
    is_valid_ = false;
    return;
  }

  // Get window.
  window_ = window_interface->Get();

  double normalization_constant(1.0);
  switch (normalization_type) {
    case kNone: {
      // nothing to do
      break;
    }
    case kPower: {
      const double power(std::inner_product(window_.begin(), window_.end(),
                                            window_.begin(), 0.0));
      normalization_constant = 1.0 / std::sqrt(power);
      break;
    }
    case kMagnitude: {
      const double magnitude(
          std::accumulate(window_.begin(), window_.end(), 0.0));
      normalization_constant = 1.0 / magnitude;
      break;
    }
    default: {
      is_valid_ = false;
      return;
    }
  }

  if (1.0 != normalization_constant) {
    std::transform(window_.begin(), window_.end(), window_.begin(),
                   [normalization_constant](double w) {
                     return w * normalization_constant;
                   });
  }
}

bool DataWindowing::Run(const std::vector<double>& data,
                        std::vector<double>* windowed_data) const {
  // Check inputs.
  if (!is_valid_ || data.size() != static_cast<std::size_t>(input_length_) ||
      NULL == windowed_data) {
    return false;
  }

  // Prepare memories.
  if (windowed_data->size() != static_cast<std::size_t>(output_length_)) {
    windowed_data->resize(output_length_);
  }

  // Apply window.
  std::transform(data.begin(), data.begin() + input_length_, window_.begin(),
                 windowed_data->begin(),
                 [](double x, double w) { return x * w; });

  // Fill zero.
  std::fill(windowed_data->begin() + input_length_, windowed_data->end(), 0.0);

  return true;
}

}  // namespace sptk
