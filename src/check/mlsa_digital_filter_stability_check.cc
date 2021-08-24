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

#include "SPTK/check/mlsa_digital_filter_stability_check.h"

#include <algorithm>  // std::copy, std::fill, std::max_element, std::transform
#include <cmath>      // std::pow, std::sqrt
#include <cstddef>    // std::size_t
#include <numeric>    // std::accumulate

namespace sptk {

MlsaDigitalFilterStabilityCheck::MlsaDigitalFilterStabilityCheck(
    int num_order, double alpha, double threshold)
    : num_order_(num_order),
      alpha_(alpha),
      threshold_(threshold),
      fast_mode_(true),
      fft_length_(0),
      modification_type_(kScaling),
      fourier_transform_(NULL),
      inverse_fourier_transform_(NULL),
      is_valid_(true) {
  if (num_order_ < 0 || !sptk::IsValidAlpha(alpha) || threshold_ <= 0.0) {
    is_valid_ = false;
    return;
  }
}

MlsaDigitalFilterStabilityCheck::MlsaDigitalFilterStabilityCheck(
    int num_order, double alpha, double threshold, int fft_length,
    ModificationType modification_type)
    : num_order_(num_order),
      alpha_(alpha),
      threshold_(threshold),
      fast_mode_(false),
      fft_length_(fft_length),
      modification_type_(modification_type),
      fourier_transform_(new RealValuedFastFourierTransform(fft_length_)),
      inverse_fourier_transform_(new InverseFastFourierTransform(fft_length_)),
      is_valid_(true) {
  if (num_order_ < 0 || !sptk::IsValidAlpha(alpha) || threshold_ <= 0.0 ||
      fft_length_ <= num_order_ ||
      kNumModificationTypes == modification_type_ ||
      !fourier_transform_->IsValid() ||
      !inverse_fourier_transform_->IsValid()) {
    is_valid_ = false;
    return;
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
  // Check inputs.
  if (!is_valid_ ||
      mel_cepstrum.size() != static_cast<std::size_t>(num_order_ + 1) ||
      NULL == is_stable || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  if (NULL != modified_mel_cepstrum &&
      modified_mel_cepstrum->size() !=
          static_cast<std::size_t>(num_order_ + 1)) {
    modified_mel_cepstrum->resize(num_order_ + 1);
  }
  if (!fast_mode_) {
    if (buffer->amplitude_.size() != static_cast<std::size_t>(fft_length_)) {
      buffer->amplitude_.resize(fft_length_);
    }
    if (buffer->fourier_transform_real_part_.size() !=
        static_cast<std::size_t>(fft_length_)) {
      buffer->fourier_transform_real_part_.resize(fft_length_);
    }
  }

  // Handle a special case.
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

  double gain(0.0);
  {
    const double* input(&(mel_cepstrum[0]));
    for (int m(0); m <= num_order_; ++m) {
      gain += input[m] * std::pow(-alpha_, m);
    }
  }

  double maximum_amplitude(0.0);
  if (fast_mode_) {
    // Usually, amplitude spectrum of human speech at zero frequency takes
    // maximum value.
    maximum_amplitude =
        std::accumulate(mel_cepstrum.begin(), mel_cepstrum.end(), -gain);
  } else {
    std::copy(mel_cepstrum.begin(), mel_cepstrum.end(),
              buffer->fourier_transform_real_part_.begin());
    std::fill(buffer->fourier_transform_real_part_.begin() + num_order_ + 1,
              buffer->fourier_transform_real_part_.end(), 0.0);

    // This line removes gain and is equivalent to the following procedure:
    // (1) apply mc2b, (2) substitute b[0] for 0, (3) apply b2mc.
    buffer->fourier_transform_real_part_[0] -= gain;

    if (!fourier_transform_->Run(&buffer->fourier_transform_real_part_,
                                 &buffer->fourier_transform_imag_part_,
                                 &buffer->fourier_transform_buffer_)) {
      return false;
    }

    std::transform(buffer->fourier_transform_real_part_.begin(),
                   buffer->fourier_transform_real_part_.end(),
                   buffer->fourier_transform_imag_part_.begin(),
                   buffer->amplitude_.begin(),
                   [](double x, double y) { return std::sqrt(x * x + y * y); });
    std::vector<double>::iterator itr(
        std::max_element(buffer->amplitude_.begin(), buffer->amplitude_.end()));
    maximum_amplitude = *itr;
  }
  if (threshold_ < maximum_amplitude) {
    *is_stable = false;
  }
  if (NULL != maximum_amplitude_of_basic_filter) {
    *maximum_amplitude_of_basic_filter = maximum_amplitude;
  }

  // Perform modification.
  if (NULL != modified_mel_cepstrum) {
    if (*is_stable) {
      std::copy(mel_cepstrum.begin(), mel_cepstrum.end(),
                modified_mel_cepstrum->begin());
    } else {
      if (fast_mode_) {
        if (kScaling == modification_type_) {
          std::copy(mel_cepstrum.begin(), mel_cepstrum.end(),
                    modified_mel_cepstrum->begin());
          double* output(&((*modified_mel_cepstrum)[0]));
          output[0] -= gain;
          for (int m(0); m <= num_order_; ++m) {
            output[m] *= threshold_ / maximum_amplitude;
          }
          output[0] += gain;
        }
      } else {
        if (kClipping == modification_type_) {
          const double* amplitude(&(buffer->amplitude_[0]));
          double* x(&(buffer->fourier_transform_real_part_[0]));
          double* y(&(buffer->fourier_transform_imag_part_[0]));
          for (int i(0); i < fft_length_; ++i) {
            if (threshold_ < amplitude[i]) {
              x[i] *= threshold_ / amplitude[i];
              y[i] *= threshold_ / amplitude[i];
            }
          }
        } else if (kScaling == modification_type_) {
          double* x(&(buffer->fourier_transform_real_part_[0]));
          double* y(&(buffer->fourier_transform_imag_part_[0]));
          for (int i(0); i < fft_length_; ++i) {
            x[i] *= threshold_ / maximum_amplitude;
            y[i] *= threshold_ / maximum_amplitude;
          }
        }

        if (!inverse_fourier_transform_->Run(
                &buffer->fourier_transform_real_part_,
                &buffer->fourier_transform_imag_part_)) {
          return false;
        }

        buffer->fourier_transform_real_part_[0] += gain;
        std::copy(buffer->fourier_transform_real_part_.begin(),
                  buffer->fourier_transform_real_part_.begin() + num_order_ + 1,
                  modified_mel_cepstrum->begin());
      }
    }
  }

  return true;
}

bool MlsaDigitalFilterStabilityCheck::Run(
    std::vector<double>* input_and_output, bool* is_stable,
    double* maximum_amplitude_of_basic_filter,
    MlsaDigitalFilterStabilityCheck::Buffer* buffer) const {
  if (NULL == input_and_output) return false;
  return Run(*input_and_output, input_and_output, is_stable,
             maximum_amplitude_of_basic_filter, buffer);
}

}  // namespace sptk
