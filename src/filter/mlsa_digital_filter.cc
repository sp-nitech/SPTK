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

#include "SPTK/filter/mlsa_digital_filter.h"

#include <algorithm>  // std::fill
#include <cmath>      // std::exp
#include <cstddef>    // std::size_t

namespace sptk {

MlsaDigitalFilter::MlsaDigitalFilter(int num_filter_order, int num_pade_order,
                                     double alpha, bool transposition)
    : num_filter_order_(num_filter_order),
      num_pade_order_(num_pade_order),
      alpha_(alpha),
      transposition_(transposition),
      is_valid_(true) {
  if (num_filter_order_ < 0 || num_pade_order_ < 0) {
    is_valid_ = false;
    return;
  }

  pade_coefficients_.resize(num_pade_order_ + 1);
  switch (num_pade_order_) {
    case 4: {
      pade_coefficients_[1] = 0.4999273;
      pade_coefficients_[2] = 0.1067005;
      pade_coefficients_[3] = 0.01170221;
      pade_coefficients_[4] = 0.0005656279;
      break;
    }
    case 5: {
      pade_coefficients_[1] = 0.4999391;
      pade_coefficients_[2] = 0.1107098;
      pade_coefficients_[3] = 0.01369984;
      pade_coefficients_[4] = 0.0009564853;
      pade_coefficients_[5] = 0.00003041721;
      break;
    }
    case 6: {
      pade_coefficients_[1] = 0.500000157834843;
      pade_coefficients_[2] = 0.113600112846183;
      pade_coefficients_[3] = 0.015133367945131;
      pade_coefficients_[4] = 0.001258740956606;
      pade_coefficients_[5] = 0.000062701416552;
      pade_coefficients_[6] = 0.000001481891776;
      break;
    }
    case 7: {
      pade_coefficients_[1] = 0.499802889651314;
      pade_coefficients_[2] = 0.115274789205577;
      pade_coefficients_[3] = 0.015997611632083;
      pade_coefficients_[4] = 0.001452640362652;
      pade_coefficients_[5] = 0.000087007832645;
      pade_coefficients_[6] = 0.000003213962732;
      pade_coefficients_[7] = 0.000000057148619;
      break;
    }
    default: {
      is_valid_ = false;
      return;
    }
  }
}

bool MlsaDigitalFilter::Run(const std::vector<double>& filter_coefficients,
                            double filter_input, double* filter_output,
                            MlsaDigitalFilter::Buffer* buffer) const {
  // check inputs
  if (!is_valid_ ||
      filter_coefficients.size() !=
          static_cast<std::size_t>(num_filter_order_ + 1) ||
      NULL == filter_output || NULL == buffer) {
    return false;
  }

  // prepare memories
  if (buffer->signals_for_basic_filter1_.size() !=
      static_cast<std::size_t>(num_pade_order_ + 1)) {
    buffer->signals_for_basic_filter1_.resize(num_pade_order_ + 1);
    std::fill(buffer->signals_for_basic_filter1_.begin(),
              buffer->signals_for_basic_filter1_.end(), 0.0);
  }
  if (buffer->signals_for_basic_filter2_.size() !=
      static_cast<std::size_t>(num_pade_order_ * (num_filter_order_ + 2))) {
    buffer->signals_for_basic_filter2_.resize(num_pade_order_ *
                                              (num_filter_order_ + 2));
    std::fill(buffer->signals_for_basic_filter2_.begin(),
              buffer->signals_for_basic_filter2_.end(), 0.0);
  }
  if (buffer->signals_for_exp_filter1_.size() !=
      static_cast<std::size_t>(num_pade_order_ + 1)) {
    buffer->signals_for_exp_filter1_.resize(num_pade_order_ + 1);
    std::fill(buffer->signals_for_exp_filter1_.begin(),
              buffer->signals_for_exp_filter1_.end(), 0.0);
  }
  if (buffer->signals_for_exp_filter2_.size() !=
      static_cast<std::size_t>(num_pade_order_ + 1)) {
    buffer->signals_for_exp_filter2_.resize(num_pade_order_ + 1);
    std::fill(buffer->signals_for_exp_filter2_.begin(),
              buffer->signals_for_exp_filter2_.end(), 0.0);
  }

  // set value
  const double gained_input(filter_input * std::exp(filter_coefficients[0]));
  if (0 == num_filter_order_) {
    *filter_output = gained_input;
    return true;
  }

  const double* b(&(filter_coefficients[0]));
  const double beta(1.0 - alpha_ * alpha_);

  // First stage
  double first_output(0.0);
  {
    double* d1(&buffer->signals_for_basic_filter1_[0]);
    double* p1(&buffer->signals_for_exp_filter1_[0]);
    double x(gained_input);
    for (int i(num_pade_order_); 0 < i; --i) {
      d1[i] = beta * p1[i - 1] + alpha_ * d1[i];
      p1[i] = d1[i] * b[1];

      const double v(p1[i] * pade_coefficients_[i]);
      x += (i % 2 == 1) ? v : -v;
      first_output += v;
    }
    p1[0] = x;
    first_output += x;
  }

  // Second stage
  double second_output(0.0);
  {
    double* p2(&buffer->signals_for_exp_filter2_[0]);
    double x(first_output);
    for (int i(num_pade_order_); 0 < i; --i) {
      const int bias((i - 1) * (num_filter_order_ + 2));
      double* d2(&buffer->signals_for_basic_filter2_[bias]);

      if (transposition_) {
        d2[num_filter_order_] = b[num_filter_order_] * p2[i - 1] +
                                alpha_ * d2[num_filter_order_ - 1];
        for (int j(num_filter_order_ - 1); 1 < j; --j) {
          d2[j] += b[j] * p2[i - 1] + alpha_ * (d2[j - 1] - d2[j + 1]);
        }
        d2[1] += alpha_ * (d2[0] - d2[2]);
        p2[i] = beta * d2[0];
        for (int j(0); j < num_filter_order_; ++j) {
          d2[j] = d2[j + 1];
        }
      } else {
        d2[0] = p2[i - 1];
        d2[1] = beta * p2[i - 1] + alpha_ * d2[1];
        double y(0.0);
        for (int j(2); j <= num_filter_order_; ++j) {
          d2[j] += alpha_ * (d2[j + 1] - d2[j - 1]);
          y += d2[j] * b[j];
        }
        p2[i] = y;
        for (int j(num_filter_order_ + 1); 1 < j; --j) {
          d2[j] = d2[j - 1];
        }
      }

      const double v(p2[i] * pade_coefficients_[i]);
      x += (i % 2 == 1) ? v : -v;
      second_output += v;
    }
    p2[0] = x;
    second_output += x;
  }

  *filter_output = second_output;

  return true;
}

}  // namespace sptk
