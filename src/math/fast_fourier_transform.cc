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

#include "SPTK/math/fast_fourier_transform.h"

#include <algorithm>  // std::copy, std::fill
#include <cmath>      // std::sin
#include <cstddef>    // std::size_t

namespace sptk {

FastFourierTransform::FastFourierTransform(int fft_length)
    : FastFourierTransform(fft_length - 1, fft_length) {
}

FastFourierTransform::FastFourierTransform(int num_order, int fft_length)
    : num_order_(num_order),
      fft_length_(fft_length),
      half_fft_length_(fft_length_ / 2),
      is_valid_(true) {
  if (num_order_ < 0 || fft_length_ <= num_order_ ||
      !IsPowerOfTwo(fft_length_)) {
    is_valid_ = false;
    return;
  }

  const int table_size(fft_length_ - fft_length_ / 4 + 1);
  const double argument(sptk::kPi / fft_length_ * 2);
  sine_table_.resize(table_size);
  for (int i(0); i < table_size; ++i) {
    sine_table_[i] = std::sin(argument * i);
  }
  sine_table_[fft_length_ / 2] = 0.0;
}

bool FastFourierTransform::Run(const std::vector<double>& real_part_input,
                               const std::vector<double>& imag_part_input,
                               std::vector<double>* real_part_output,
                               std::vector<double>* imag_part_output) const {
  // Check inputs.
  if (!is_valid_ ||
      real_part_input.size() != static_cast<std::size_t>(num_order_ + 1) ||
      imag_part_input.size() != static_cast<std::size_t>(num_order_ + 1) ||
      NULL == real_part_output || NULL == imag_part_output) {
    return false;
  }

  // Prepare memories.
  if (real_part_output->size() != static_cast<std::size_t>(fft_length_)) {
    real_part_output->resize(fft_length_);
  }
  if (imag_part_output->size() != static_cast<std::size_t>(fft_length_)) {
    imag_part_output->resize(fft_length_);
  }

  // Copy inputs and fill zero.
  std::copy(real_part_input.begin(), real_part_input.end(),
            real_part_output->begin());
  std::fill(real_part_output->begin() + real_part_input.size(),
            real_part_output->end(), 0.0);
  std::copy(imag_part_input.begin(), imag_part_input.end(),
            imag_part_output->begin());
  std::fill(imag_part_output->begin() + imag_part_input.size(),
            imag_part_output->end(), 0.0);

  double* x(&((*real_part_output)[0]));
  double* y(&((*imag_part_output)[0]));

  {
    int lix(fft_length_);
    int lmx(half_fft_length_);
    int lf(1);
    while (1 < lmx) {
      const double* sinp(&(sine_table_[0]));
      const double* cosp(&(sine_table_[0]) + fft_length_ / 4);
      for (int i(0); i < lmx; ++i) {
        double* xpi(&(x[i]));
        double* ypi(&(y[i]));
        for (int li(lix); li <= fft_length_; li += lix) {
          const double t1(*(xpi) - *(xpi + lmx));
          const double t2(*(ypi) - *(ypi + lmx));
          *(xpi) += *(xpi + lmx);
          *(ypi) += *(ypi + lmx);
          *(xpi + lmx) = *cosp * t1 + *sinp * t2;
          *(ypi + lmx) = *cosp * t2 - *sinp * t1;
          xpi += lix;
          ypi += lix;
        }
        sinp += lf;
        cosp += lf;
      }
      lix = lmx;
      lmx /= 2;
      lf *= 2;
    }
  }

  {
    double* xp(x);
    double* yp(y);
    for (int li(0); li < half_fft_length_; ++li) {
      const double t1(*(xp) - *(xp + 1));
      const double t2(*(yp) - *(yp + 1));
      *(xp) += *(xp + 1);
      *(yp) += *(yp + 1);
      *(xp + 1) = t1;
      *(yp + 1) = t2;
      xp += 2;
      yp += 2;
    }
  }

  // Bit reversal.
  {
    double* xp(x);
    double* yp(y);
    const int dec_fft_length(fft_length_ - 1);
    for (int lmx(0), j(0); lmx < dec_fft_length; ++lmx) {
      const int lmxj(lmx - j);
      if (lmxj < 0) {
        const double t1(*(xp));
        const double t2(*(yp));
        *(xp) = *(xp + lmxj);
        *(yp) = *(yp + lmxj);
        *(xp + lmxj) = t1;
        *(yp + lmxj) = t2;
      }

      int li(half_fft_length_);
      while (li <= j) {
        j -= li;
        li /= 2;
      }
      j += li;
      xp = x + j;
      yp = y + j;
    }
  }

  return true;
}

bool FastFourierTransform::Run(std::vector<double>* real_part,
                               std::vector<double>* imag_part) const {
  if (NULL == real_part || NULL == imag_part) return false;
  return Run(*real_part, *imag_part, real_part, imag_part);
}

}  // namespace sptk
