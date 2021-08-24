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

#include "SPTK/window/cosine_window.h"

#include <cmath>  // std::cos

namespace sptk {

CosineWindow::CosineWindow(int window_length, const std::vector<double>& alpha,
                           bool periodic)
    : window_length_(window_length), periodic_(periodic), is_valid_(true) {
  if (window_length_ <= 0 || alpha.empty()) {
    is_valid_ = false;
    return;
  }

  window_.resize(window_length_);
  if (1 == window_length_) {
    window_[0] = 1.0;
    return;
  }

  const int alpha_size(static_cast<int>(alpha.size()));
  const double m(kTwoPi / (periodic_ ? window_length_ : window_length_ - 1));
  double* window(&(window_[0]));
  for (int i(0); i < window_length_; ++i) {
    double sum(alpha[0]);
    int sign(-1);
    for (int k(1); k < alpha_size; ++k) {
      sum += sign * alpha[k] * std::cos(i * k * m);
      sign *= -1;
    }
    window[i] = sum;
  }
}

}  // namespace sptk
