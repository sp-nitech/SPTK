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

#include "SPTK/window/standard_window.h"

#include <algorithm>  // std::fill
#include <cmath>      // std::round

#include "SPTK/window/cosine_window.h"

namespace {

void MakeBartlett(bool periodic, std::vector<double>* window) {
  const int size(static_cast<int>(window->size()));
  const int center(sptk::IsEven(size) ? size / 2 : (size + 1) / 2);
  const double slope(2.0 / (periodic ? size : size - 1));
  double* w(&((*window)[0]));
  for (int i(0); i < center; ++i) {
    w[i] = slope * i;
  }
  for (int i(center); i < size; ++i) {
    w[i] = 2.0 - slope * i;
  }
}

void MakeBlackman(bool periodic, std::vector<double>* window) {
  sptk::CosineWindow maker(static_cast<int>(window->size()), {0.42, 0.5, 0.08},
                           periodic);
  *window = maker.Get();
}

void MakeBlackmanHarris(bool periodic, std::vector<double>* window) {
  sptk::CosineWindow maker(static_cast<int>(window->size()),
                           {0.35875, 0.48829, 0.14128, 0.01168}, periodic);
  *window = maker.Get();
}

void MakeBlackmanNuttall(bool periodic, std::vector<double>* window) {
  sptk::CosineWindow maker(static_cast<int>(window->size()),
                           {0.3635819, 0.4891775, 0.1365995, 0.0106411},
                           periodic);
  *window = maker.Get();
}

void MakeFlatTop(bool periodic, std::vector<double>* window) {
  sptk::CosineWindow maker(
      static_cast<int>(window->size()),
      {0.21557895, 0.41663158, 0.277263158, 0.083578947, 0.006947368},
      periodic);
  *window = maker.Get();
}

void MakeHamming(bool periodic, std::vector<double>* window) {
  sptk::CosineWindow maker(static_cast<int>(window->size()), {0.54, 0.46},
                           periodic);
  *window = maker.Get();
}

void MakeHanning(bool periodic, std::vector<double>* window) {
  sptk::CosineWindow maker(static_cast<int>(window->size()), {0.5, 0.5},
                           periodic);
  *window = maker.Get();
}

void MakeNuttall(bool periodic, std::vector<double>* window) {
  sptk::CosineWindow maker(static_cast<int>(window->size()),
                           {0.3635819, 0.4891775, 0.1365995, 0.0106411},
                           periodic);
  *window = maker.Get();
}

void MakeRectangular(std::vector<double>* window) {
  std::fill(window->begin(), window->end(), 1.0);
}

void MakeTrapezoidal(bool periodic, std::vector<double>* window) {
  const int size(static_cast<int>(window->size()));
  const int quarter_size(static_cast<int>(std::round(0.25 * size)));
  const double slope(4.0 / (periodic ? size : size - 1));
  double* w(&((*window)[0]));
  for (int i(0); i < quarter_size; ++i) {
    w[i] = slope * i;
  }
  std::fill(window->begin() + quarter_size, window->end() - quarter_size, 1.0);
  for (int i(size - quarter_size); i < size; ++i) {
    w[i] = 4.0 - slope * i;
  }
}

}  // namespace

namespace sptk {

StandardWindow::StandardWindow(int window_length, WindowType window_type,
                               bool periodic)
    : window_length_(window_length),
      window_type_(window_type),
      periodic_(periodic),
      is_valid_(true) {
  if (window_length_ <= 0) {
    is_valid_ = false;
    return;
  }

  window_.resize(window_length);
  if (1 == window_length_) {
    window_[0] = 1.0;
    return;
  }

  switch (window_type_) {
    case WindowType::kBartlett: {
      MakeBartlett(periodic_, &window_);
      break;
    }
    case WindowType::kBlackman: {
      MakeBlackman(periodic_, &window_);
      break;
    }
    case WindowType::kBlackmanHarris: {
      MakeBlackmanHarris(periodic_, &window_);
      break;
    }
    case WindowType::kBlackmanNuttall: {
      MakeBlackmanNuttall(periodic_, &window_);
      break;
    }
    case WindowType::kFlatTop: {
      MakeFlatTop(periodic_, &window_);
      break;
    }
    case WindowType::kHamming: {
      MakeHamming(periodic_, &window_);
      break;
    }
    case WindowType::kHanning: {
      MakeHanning(periodic_, &window_);
      break;
    }
    case WindowType::kNuttall: {
      MakeNuttall(periodic_, &window_);
      break;
    }
    case WindowType::kRectangular: {
      MakeRectangular(&window_);
      break;
    }
    case WindowType::kTrapezoidal: {
      MakeTrapezoidal(periodic_, &window_);
      break;
    }
  }
}

}  // namespace sptk
