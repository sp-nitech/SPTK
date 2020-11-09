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

#include "SPTK/window/standard_window.h"

#include <algorithm>  // std::fill
#include <cmath>      // std::round

#include "SPTK/window/cosine_window.h"

namespace {

void MakeBartlett(bool periodic, std::vector<double>* window) {
  const int size(window->size());
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
  sptk::CosineWindow maker(window->size(), {0.42, 0.5, 0.08}, periodic);
  *window = maker.Get();
}

void MakeBlackmanHarris(bool periodic, std::vector<double>* window) {
  sptk::CosineWindow maker(window->size(), {0.35875, 0.48829, 0.14128, 0.01168},
                           periodic);
  *window = maker.Get();
}

void MakeBlackmanNuttall(bool periodic, std::vector<double>* window) {
  sptk::CosineWindow maker(
      window->size(), {0.3635819, 0.4891775, 0.1365995, 0.0106411}, periodic);
  *window = maker.Get();
}

void MakeFlatTop(bool periodic, std::vector<double>* window) {
  sptk::CosineWindow maker(window->size(), {0.21557895, 0.41663158, 0.277263158,
                                            0.083578947, 0.006947368},
                           periodic);
  *window = maker.Get();
}

void MakeHamming(bool periodic, std::vector<double>* window) {
  sptk::CosineWindow maker(window->size(), {0.54, 0.46}, periodic);
  *window = maker.Get();
}

void MakeHanning(bool periodic, std::vector<double>* window) {
  sptk::CosineWindow maker(window->size(), {0.5, 0.5}, periodic);
  *window = maker.Get();
}

void MakeNuttall(bool periodic, std::vector<double>* window) {
  sptk::CosineWindow maker(
      window->size(), {0.3635819, 0.4891775, 0.1365995, 0.0106411}, periodic);
  *window = maker.Get();
}

void MakeRectangular(std::vector<double>* window) {
  std::fill(window->begin(), window->end(), 1.0);
}

void MakeTrapezoidal(bool periodic, std::vector<double>* window) {
  const int size(window->size());
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
