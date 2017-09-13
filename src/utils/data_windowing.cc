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
//                1996-2017  Nagoya Institute of Technology          //
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

#include "SPTK/utils/data_windowing.h"

#include <algorithm>   // std::fill, std::transform
#include <cmath>       // std::cos, std::round, std::sqrt
#include <cstddef>     // std::size_t
#include <functional>  // std::bind1st, std::multiplies
#include <numeric>     // std::accumulate, std::inner_product

namespace sptk {

DataWindowing::DataWindowing(int num_input_order, int num_output_order,
                             NormalizationType normalization_type,
                             WindowType window_type)
    : num_input_order_(num_input_order),
      num_output_order_(num_output_order),
      is_valid_(true) {
  if (num_input_order_ < 0 || num_output_order_ < num_input_order_) {
    is_valid_ = false;
    return;
  }

  window_.resize(num_input_order_ + 1);
  switch (window_type) {
    case kBlackman: {
      CreateBlackmanWindow();
      break;
    }
    case kHamming: {
      CreateHammingWindow();
      break;
    }
    case kHanning: {
      CreateHanningWindow();
      break;
    }
    case kBartlett: {
      CreateBartlettWindow();
      break;
    }
    case kTrapezoidal: {
      CreateTrapezoidalWindow();
      break;
    }
    case kRectangular: {
      CreateRectangularWindow();
      break;
    }
    default: {
      is_valid_ = false;
      return;
    }
  }

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

  // normalize
  if (1.0 != normalization_constant) {
    std::transform(
        window_.begin(), window_.end(), window_.begin(),
        std::bind1st(std::multiplies<double>(), normalization_constant));
  }
}

bool DataWindowing::Run(const std::vector<double>& data_sequence,
                        std::vector<double>* windowed_data_sequence) const {
  // check inputs
  const int input_length(num_input_order_ + 1);
  if (!is_valid_ ||
      data_sequence.size() != static_cast<std::size_t>(input_length) ||
      NULL == windowed_data_sequence) {
    return false;
  }

  // prepare memory
  const int output_length(num_output_order_ + 1);
  if (windowed_data_sequence->size() <
      static_cast<std::size_t>(output_length)) {
    windowed_data_sequence->resize(output_length);
  }

  // apply window
  std::transform(data_sequence.begin(), data_sequence.begin() + input_length,
                 window_.begin(), windowed_data_sequence->begin(),
                 std::multiplies<double>());

  // fill zero
  std::fill(windowed_data_sequence->begin() + input_length,
            windowed_data_sequence->end(), 0.0);

  return true;
}

void DataWindowing::CreateBlackmanWindow() {
  const int window_size(window_.size());
  const double argument(sptk::kTwoPi / (window_size - 1));
  for (int i(0); i < window_size; ++i) {
    const double x(argument * i);
    window_[i] = 0.42 - 0.50 * std::cos(x) + 0.08 * std::cos(2.0 * x);
  }
}

void DataWindowing::CreateHammingWindow() {
  const int window_size(window_.size());
  const double argument(sptk::kTwoPi / (window_size - 1));
  for (int i(0); i < window_size; ++i) {
    window_[i] = 0.54 - 0.46 * std::cos(argument * i);
  }
}

void DataWindowing::CreateHanningWindow() {
  const int window_size(window_.size());
  const double argument(sptk::kTwoPi / (window_size - 1));
  for (int i(0); i < window_size; ++i) {
    window_[i] = 0.50 - 0.50 * std::cos(argument * i);
  }
}

void DataWindowing::CreateBartlettWindow() {
  const int window_size(window_.size());
  const int half_window_size(static_cast<int>(std::round(0.5 * window_size)));
  const double slope(2.0 / (window_size - 1));
  for (int i(0); i < half_window_size; ++i) {
    window_[i] = slope * i;
  }
  for (int i(half_window_size); i < window_size; ++i) {
    window_[i] = 2.0 - slope * i;
  }
}

void DataWindowing::CreateTrapezoidalWindow() {
  const int window_size(window_.size());
  const int quarter_window_size(
      static_cast<int>(std::round(0.25 * window_size)));
  const double slope(4.0 / (window_size - 1));
  for (int i(0); i < quarter_window_size; ++i) {
    window_[i] = slope * i;
  }
  std::fill(window_.begin() + quarter_window_size,
            window_.end() - quarter_window_size, 1.0);
  for (int i(window_size - quarter_window_size); i < window_size; ++i) {
    window_[i] = 4.0 - slope * i;
  }
}

void DataWindowing::CreateRectangularWindow() {
  std::fill(window_.begin(), window_.end(), 1.0);
}

}  // namespace sptk
