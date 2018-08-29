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

#include "SPTK/utils/mlsa_digital_filter_stability_check.h"

#include <algorithm>  // std::copy, std::fill
#include <cmath>      // std::pow, std::sqrt
#include <numeric>    // std::accumulate

namespace sptk {

MlsaDigitalFilterStabilityCheck::MlsaDigitalFilterStabilityCheck(
    int num_order, double alpha, double threshold, bool fast_mode,
    int fft_length, ModificationType modification_type)
    : num_order_(num_order),
      alpha_(alpha),
      threshold_(threshold),
      fast_mode_(fast_mode),
      modification_type_(modification_type),
      fourier_transform_(fast_mode ? NULL
                                   : new FastFourierTransformForRealSequence(
                                         fft_length - 1, fft_length)),
      inverse_fourier_transform_(fast_mode ? NULL
                                           : new InverseFastFourierTransform(
                                                 fft_length - 1, fft_length)),
      is_valid_(true) {
  if (num_order_ < 0 || threshold_ <= 0.0) {
    is_valid_ = false;
  }

  if (!fast_mode && (fft_length <= num_order_ ||
                     kNumModificationTypes == modification_type_ ||
                     !fourier_transform_->IsValid() ||
                     !inverse_fourier_transform_->IsValid())) {
    is_valid_ = false;
  }
}

MlsaDigitalFilterStabilityCheck::~MlsaDigitalFilterStabilityCheck() {
  delete fourier_transform_;
  delete inverse_fourier_transform_;
}

bool MlsaDigitalFilterStabilityCheck::Run(
    const std::vector<double>& mel_cepstrum,
    std::vector<double>* modified_mel_cepstrum, bool* is_stable,
    double* maximum_amplitude_of_basic_filter,
    MlsaDigitalFilterStabilityCheck::Buffer* buffer) const {
  if (!is_valid_ ||
      mel_cepstrum.size() != static_cast<std::size_t>(num_order_ + 1) ||
      NULL == is_stable || NULL == buffer) {
    return false;
  }

  if (NULL != modified_mel_cepstrum &&
      modified_mel_cepstrum->size() !=
          static_cast<std::size_t>(num_order_ + 1)) {
    modified_mel_cepstrum->resize(num_order_ + 1);
  }

  *is_stable = true;
  if (0 == num_order_) {
    if (NULL != modified_mel_cepstrum) {
      (*modified_mel_cepstrum)[0] = mel_cepstrum[0];
    }
    if (NULL != maximum_amplitude_of_basic_filter) {
      *maximum_amplitude_of_basic_filter = 0.0;
    }
    return true;
  }

  const int fft_length(GetFftLength());
  if (!fast_mode_) {
    if (buffer->amplitude_.size() != static_cast<std::size_t>(fft_length)) {
      buffer->amplitude_.resize(fft_length);
    }
    if (buffer->fourier_transform_real_part_input_.size() !=
        static_cast<std::size_t>(fft_length)) {
      buffer->fourier_transform_real_part_input_.resize(fft_length);
    }
  }

  double gain(0.0);
  {
    const double* input(&(mel_cepstrum[0]));
    for (int i(0); i <= num_order_; ++i) {
      gain += input[i] * std::pow(-alpha_, i);
    }
  }

  double maximum_amplitude(0.0);
  if (fast_mode_) {
    // usually, amplitude spectrum of human speech at zero frequency takes
    // maximum value
    maximum_amplitude =
        std::accumulate(mel_cepstrum.begin(), mel_cepstrum.end(), -gain);
  } else {
    std::copy(mel_cepstrum.begin(), mel_cepstrum.end(),
              buffer->fourier_transform_real_part_input_.begin());
    std::fill(
        buffer->fourier_transform_real_part_input_.begin() + num_order_ + 1,
        buffer->fourier_transform_real_part_input_.end(), 0.0);

    // this line removes gain and is equivalent to the following procedure:
    // (1) apply mc2b, (2) substitute b[0] for 0, (3) apply b2mc.
    buffer->fourier_transform_real_part_input_[0] -= gain;

    if (!fourier_transform_->Run(
            buffer->fourier_transform_real_part_input_,
            &buffer->fourier_transform_real_part_output_,
            &buffer->fourier_transform_imaginary_part_output_,
            &buffer->fourier_transform_buffer_)) {
      return false;
    }

    double* amplitude(&(buffer->amplitude_[0]));
    double* x(&(buffer->fourier_transform_real_part_output_[0]));
    double* y(&(buffer->fourier_transform_imaginary_part_output_[0]));
    for (int i(0); i < fft_length; ++i) {
      amplitude[i] = std::sqrt(x[i] * x[i] + y[i] * y[i]);
      if (maximum_amplitude < amplitude[i]) {
        maximum_amplitude = amplitude[i];
      }
    }
  }
  if (threshold_ < maximum_amplitude) {
    *is_stable = false;
  }

  if (NULL != maximum_amplitude_of_basic_filter) {
    *maximum_amplitude_of_basic_filter = maximum_amplitude;
  }

  if (NULL != modified_mel_cepstrum) {
    if (*is_stable) {
      std::copy(mel_cepstrum.begin(), mel_cepstrum.end(),
                modified_mel_cepstrum->begin());
    } else {
      if (fast_mode_) {
        std::copy(mel_cepstrum.begin(), mel_cepstrum.end(),
                  modified_mel_cepstrum->begin());
        double* output(&((*modified_mel_cepstrum)[0]));
        output[0] -= gain;
        for (int i(0); i <= num_order_; ++i) {
          output[i] *= threshold_ / maximum_amplitude;
        }
        output[0] += gain;
      } else {
        if (kClipping == modification_type_) {
          const double* amplitude(&(buffer->amplitude_[0]));
          double* x(&(buffer->fourier_transform_real_part_output_[0]));
          double* y(&(buffer->fourier_transform_imaginary_part_output_[0]));
          for (int i(0); i < fft_length; ++i) {
            if (threshold_ < amplitude[i]) {
              x[i] *= threshold_ / amplitude[i];
              y[i] *= threshold_ / amplitude[i];
            }
          }
        } else if (kScaling == modification_type_) {
          double* x(&(buffer->fourier_transform_real_part_output_[0]));
          double* y(&(buffer->fourier_transform_imaginary_part_output_[0]));
          for (int i(0); i < fft_length; ++i) {
            x[i] *= threshold_ / maximum_amplitude;
            y[i] *= threshold_ / maximum_amplitude;
          }
        }

        if (!inverse_fourier_transform_->Run(
                buffer->fourier_transform_real_part_output_,
                buffer->fourier_transform_imaginary_part_output_,
                &buffer->fourier_transform_real_part_input_,
                &buffer->fourier_transform_imaginary_part_input_)) {
          return false;
        }

        buffer->fourier_transform_real_part_input_[0] += gain;
        std::copy(
            buffer->fourier_transform_real_part_input_.begin(),
            buffer->fourier_transform_real_part_input_.begin() + num_order_ + 1,
            modified_mel_cepstrum->begin());
      }
    }
  }

  return true;
}

}  // namespace sptk
