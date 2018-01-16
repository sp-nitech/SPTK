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
//                1996-2018  Nagoya Institute of Technology          //
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

#include "SPTK/analyzer/adaptive_mel_cepstral_analyzer.h"

#include <algorithm>   // std::fill, std::transform
#include <cmath>       // std::log
#include <cstddef>     // std::size_t
#include <functional>  // std::bind1st, std::multiplies

namespace sptk {

AdaptiveMelCepstralAnalyzer::AdaptiveMelCepstralAnalyzer(
    int num_order, int num_pade_order, double alpha, double minimum_epsilon,
    double momentum, double forgetting_factor, double step_size_factor)
    : minimum_epsilon_(minimum_epsilon),
      momentum_(momentum),
      forgetting_factor_(forgetting_factor),
      step_size_factor_(step_size_factor),
      mlsa_digital_filter_(num_order, num_pade_order, alpha, false),
      mlsa_digital_filter_coefficients_to_mel_cepstrum_(num_order, alpha),
      is_valid_(true) {
  if (minimum_epsilon_ <= 0.0 || momentum_ < 0.0 || 1.0 <= momentum_ ||
      forgetting_factor_ < 0.0 || 1.0 <= forgetting_factor_ ||
      step_size_factor_ <= 0.0 || 1.0 <= step_size_factor_ ||
      !mlsa_digital_filter_.IsValid() ||
      !mlsa_digital_filter_coefficients_to_mel_cepstrum_.IsValid()) {
    is_valid_ = false;
  }
}

bool AdaptiveMelCepstralAnalyzer::Run(
    double input_signal, double* prediction_error,
    std::vector<double>* mel_cepstrum,
    AdaptiveMelCepstralAnalyzer::Buffer* buffer) const {
  // check inputs
  if (!is_valid_ || NULL == prediction_error || NULL == mel_cepstrum ||
      NULL == buffer) {
    return false;
  }

  // prepare memories
  const int num_order(GetNumOrder());
  if (buffer->mlsa_digital_filter_coefficients_.size() <
      static_cast<std::size_t>(num_order + 1)) {
    buffer->mlsa_digital_filter_coefficients_.resize(num_order + 1);
    std::fill(buffer->mlsa_digital_filter_coefficients_.begin(),
              buffer->mlsa_digital_filter_coefficients_.end(), 0.0);
  }
  if (buffer->inverse_mlsa_digital_filter_coefficients_.size() <
      static_cast<std::size_t>(num_order + 1)) {
    buffer->inverse_mlsa_digital_filter_coefficients_.resize(num_order + 1);
    buffer->inverse_mlsa_digital_filter_coefficients_[0] = 0.0;
  }
  if (buffer->buffer_for_phi_digital_filter_.size() <
      static_cast<std::size_t>(num_order + 1)) {
    buffer->buffer_for_phi_digital_filter_.resize(num_order + 1);
    std::fill(buffer->buffer_for_phi_digital_filter_.begin(),
              buffer->buffer_for_phi_digital_filter_.end(), 0.0);
  }
  if (buffer->gradient_.size() < static_cast<std::size_t>(num_order)) {
    buffer->gradient_.resize(num_order);
    std::fill(buffer->gradient_.begin(), buffer->gradient_.end(), 0.0);
  }

  // apply inverse MLSA digital filter
  std::transform(buffer->mlsa_digital_filter_coefficients_.begin() + 1,
                 buffer->mlsa_digital_filter_coefficients_.end(),
                 buffer->inverse_mlsa_digital_filter_coefficients_.begin() + 1,
                 std::bind1st(std::multiplies<double>(), -1.0));
  double curr_prediction_error;
  if (!mlsa_digital_filter_.Run(
          buffer->inverse_mlsa_digital_filter_coefficients_, input_signal,
          &curr_prediction_error, &buffer->buffer_for_mlsa_digital_filter_)) {
    return false;
  }

  // apply phi digital filter
  {
    const double alpha(GetAlpha());
    double* d(&buffer->buffer_for_phi_digital_filter_[0]);
    d[0] =
        alpha * d[0] + (1.0 - alpha * alpha) * buffer->prev_prediction_error_;
    for (int i(1); i < num_order; ++i) {
      d[i] += alpha * (d[i + 1] - d[i - 1]);
    }
    for (int i(num_order); 0 < i; --i) {
      d[i] = d[i - 1];
    }
  }

  // update epsilon
  double curr_epsilon(forgetting_factor_ * buffer->prev_epsilon_ +
                      (1.0 - forgetting_factor_) * curr_prediction_error *
                          curr_prediction_error);
  if (curr_epsilon < minimum_epsilon_) {
    curr_epsilon = minimum_epsilon_;
  }

  // update coefficients of MLSA digital filter
  {
    const double c(2.0 * (1.0 - momentum_) * curr_prediction_error);
    const double mu(step_size_factor_ / (num_order * curr_epsilon));
    const double* error(&buffer->buffer_for_phi_digital_filter_[1]);
    double* gradient(&buffer->gradient_[0]);
    double* b(&buffer->mlsa_digital_filter_coefficients_[1]);
    buffer->mlsa_digital_filter_coefficients_[0] = 0.5 * std::log(curr_epsilon);
    for (int i(0); i < num_order; ++i) {
      gradient[i] = momentum_ * gradient[i] - c * error[i];
      b[i] -= mu * gradient[i];
    }
  }

  // store
  *prediction_error = curr_prediction_error;
  buffer->prev_prediction_error_ = curr_prediction_error;
  buffer->prev_epsilon_ = curr_epsilon;

  if (!mlsa_digital_filter_coefficients_to_mel_cepstrum_.Run(
          buffer->mlsa_digital_filter_coefficients_, mel_cepstrum)) {
    return false;
  }

  return true;
}

}  // namespace sptk
