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
//                1996-2021  Nagoya Institute of Technology          //
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

#include "SPTK/filter/second_order_digital_filter.h"

#include <cmath>   // std::cos, std::exp
#include <vector>  // std::vector

namespace {

std::vector<double> GetFilterCoefficients(double frequency, double bandwidth,
                                          double sampling_rate) {
  const double radius(std::exp(-sptk::kPi * bandwidth / sampling_rate));
  const double angle(2.0 * sptk::kPi * frequency / sampling_rate);
  return {1.0, -2.0 * radius * std::cos(angle), radius * radius};
}

}  // namespace

namespace sptk {

SecondOrderDigitalFilter::SecondOrderDigitalFilter(FilterType filter_type,
                                                   double frequency,
                                                   double bandwidth,
                                                   double sampling_rate)
    : filter_((kPole == filter_type)
                  ? GetFilterCoefficients(frequency, bandwidth, sampling_rate)
                  : std::vector<double>({1.0}),
              (kPole == filter_type)
                  ? std::vector<double>({1.0})
                  : GetFilterCoefficients(frequency, bandwidth, sampling_rate)),
      is_valid_(true) {
  const double nyquist_frequency(0.5 * sampling_rate);
  if (frequency <= 0.0 || nyquist_frequency <= frequency || bandwidth <= 0.0) {
    is_valid_ = false;
    return;
  }
}

SecondOrderDigitalFilter::SecondOrderDigitalFilter(double pole_frequency,
                                                   double pole_bandwidth,
                                                   double zero_frequency,
                                                   double zero_bandwidth,
                                                   double sampling_rate)
    : filter_(
          GetFilterCoefficients(pole_frequency, pole_bandwidth, sampling_rate),
          GetFilterCoefficients(zero_frequency, zero_bandwidth, sampling_rate)),
      is_valid_(true) {
  const double nyquist_frequency(0.5 * sampling_rate);
  if (pole_frequency <= 0.0 || nyquist_frequency <= pole_frequency ||
      pole_bandwidth <= 0.0 || zero_frequency <= 0.0 ||
      nyquist_frequency <= zero_frequency || zero_bandwidth <= 0.0 ||
      !filter_.IsValid()) {
    is_valid_ = false;
    return;
  }
}

bool SecondOrderDigitalFilter::Run(
    double input, double* output,
    SecondOrderDigitalFilter::Buffer* buffer) const {
  if (!is_valid_ || NULL == output || NULL == buffer) {
    return false;
  }
  return filter_.Run(input, output, &(buffer->buffer));
}

bool SecondOrderDigitalFilter::Run(
    double* input_and_output, SecondOrderDigitalFilter::Buffer* buffer) const {
  if (NULL == input_and_output) return false;
  return Run(*input_and_output, input_and_output, buffer);
}

}  // namespace sptk
