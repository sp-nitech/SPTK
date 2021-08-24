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

#include "SPTK/conversion/mel_generalized_cepstrum_to_mel_generalized_cepstrum.h"

#include <algorithm>  // std::copy, std::transform
#include <cstddef>    // std::size_t

#include "SPTK/conversion/generalized_cepstrum_gain_normalization.h"
#include "SPTK/conversion/generalized_cepstrum_inverse_gain_normalization.h"
#include "SPTK/math/frequency_transform.h"

namespace {

class GainNormalizationModule
    : public sptk::MelGeneralizedCepstrumToMelGeneralizedCepstrum::
          ModuleInterface {
 public:
  GainNormalizationModule(int num_order, double gamma)
      : generalized_cepstrum_gain_normalization_(num_order, gamma) {
  }
  virtual bool IsValid() const {
    return generalized_cepstrum_gain_normalization_.IsValid();
  }
  virtual bool Run(
      const std::vector<double>& input, std::vector<double>* output,
      sptk::FrequencyTransform::Buffer* frequency_transform_buffer) const {
    if (!generalized_cepstrum_gain_normalization_.Run(input, output)) {
      return false;
    }
    output->resize(generalized_cepstrum_gain_normalization_.GetNumOrder() + 1);
    return true;
  }

 private:
  const sptk::GeneralizedCepstrumGainNormalization
      generalized_cepstrum_gain_normalization_;
  DISALLOW_COPY_AND_ASSIGN(GainNormalizationModule);
};

class InverseGainNormalizationModule
    : public sptk::MelGeneralizedCepstrumToMelGeneralizedCepstrum::
          ModuleInterface {
 public:
  InverseGainNormalizationModule(int num_order, double gamma)
      : generalized_cepstrum_inverse_gain_normalization_(num_order, gamma) {
  }
  virtual bool IsValid() const {
    return generalized_cepstrum_inverse_gain_normalization_.IsValid();
  }
  virtual bool Run(
      const std::vector<double>& input, std::vector<double>* output,
      sptk::FrequencyTransform::Buffer* frequency_transform_buffer) const {
    if (!generalized_cepstrum_inverse_gain_normalization_.Run(input, output)) {
      return false;
    }
    output->resize(
        generalized_cepstrum_inverse_gain_normalization_.GetNumOrder() + 1);
    return true;
  }

 private:
  const sptk::GeneralizedCepstrumInverseGainNormalization
      generalized_cepstrum_inverse_gain_normalization_;
  DISALLOW_COPY_AND_ASSIGN(InverseGainNormalizationModule);
};

class FrequencyTransformModule
    : public sptk::MelGeneralizedCepstrumToMelGeneralizedCepstrum::
          ModuleInterface {
 public:
  FrequencyTransformModule(int num_input_order, int num_output_order,
                           double alpha_transform)
      : frequency_transform_(num_input_order, num_output_order,
                             alpha_transform) {
  }
  virtual bool IsValid() const {
    return frequency_transform_.IsValid();
  }
  virtual bool Run(
      const std::vector<double>& input, std::vector<double>* output,
      sptk::FrequencyTransform::Buffer* frequency_transform_buffer) const {
    if (!frequency_transform_.Run(input, output, frequency_transform_buffer)) {
      return false;
    }
    output->resize(frequency_transform_.GetNumOutputOrder() + 1);
    return true;
  }

 private:
  const sptk::FrequencyTransform frequency_transform_;
  DISALLOW_COPY_AND_ASSIGN(FrequencyTransformModule);
};

class MelGeneralizedCepstrumToMelGeneralizedCepstrumModule
    : public sptk::MelGeneralizedCepstrumToMelGeneralizedCepstrum::
          ModuleInterface {
 public:
  MelGeneralizedCepstrumToMelGeneralizedCepstrumModule(int num_input_order,
                                                       int num_output_order,
                                                       double input_gamma,
                                                       double output_gamma)
      : num_input_order_(num_input_order),
        num_output_order_(num_output_order),
        input_gamma_(input_gamma),
        output_gamma_(output_gamma) {
  }
  virtual bool IsValid() const {
    return true;
  }
  virtual bool Run(
      const std::vector<double>& input, std::vector<double>* output,
      sptk::FrequencyTransform::Buffer* frequency_transform_buffer) const {
    if (output->size() != static_cast<std::size_t>(num_output_order_ + 1)) {
      output->resize(num_output_order_ + 1);
    }
    const double* c1(&(input)[0]);
    double* c2(&((*output)[0]));

    c2[0] = c1[0];
    for (int m(1); m <= num_output_order_; ++m) {
      double ss1(0.0), ss2(0.0);
      const int min(num_input_order_ < m ? num_input_order_ : m - 1);
      for (int k(1); k <= min; ++k) {
        const int mk(m - k);
        const double cc(c1[k] * c2[mk]);
        ss2 += k * cc;
        ss1 += mk * cc;
      }

      if (m <= num_input_order_) {
        c2[m] = c1[m] + (output_gamma_ * ss2 - input_gamma_ * ss1) / m;
      } else {
        c2[m] = (output_gamma_ * ss2 - input_gamma_ * ss1) / m;
      }
    }
    return true;
  }

 private:
  const int num_input_order_;
  const int num_output_order_;
  const double input_gamma_;
  const double output_gamma_;
  DISALLOW_COPY_AND_ASSIGN(
      MelGeneralizedCepstrumToMelGeneralizedCepstrumModule);
};

class GammaDivisionModule
    : public sptk::MelGeneralizedCepstrumToMelGeneralizedCepstrum::
          ModuleInterface {
 public:
  GammaDivisionModule(int num_order, double gamma)
      : num_order_(num_order), gamma_(gamma) {
  }
  virtual bool IsValid() const {
    return true;
  }
  virtual bool Run(
      const std::vector<double>& input, std::vector<double>* output,
      sptk::FrequencyTransform::Buffer* frequency_transform_buffer) const {
    if (output->size() != static_cast<std::size_t>(num_order_ + 1)) {
      output->resize(num_order_ + 1);
    }
    // c[0] is not changed
    *(output->begin()) = *(input.begin());
    // c[1-m] /= g
    std::transform(++(input.begin()), input.end(), ++(output->begin()),
                   [this](double c) { return c / gamma_; });
    return true;
  }

 private:
  const int num_order_;
  const double gamma_;
  DISALLOW_COPY_AND_ASSIGN(GammaDivisionModule);
};

class GammaMultiplicationModule
    : public sptk::MelGeneralizedCepstrumToMelGeneralizedCepstrum::
          ModuleInterface {
 public:
  GammaMultiplicationModule(int num_order, double gamma)
      : num_order_(num_order), gamma_(gamma) {
  }
  virtual bool IsValid() const {
    return true;
  }
  virtual bool Run(
      const std::vector<double>& input, std::vector<double>* output,
      sptk::FrequencyTransform::Buffer* frequency_transform_buffer) const {
    if (output->size() != static_cast<std::size_t>(num_order_ + 1)) {
      output->resize(num_order_ + 1);
    }
    // copy c[0]
    *(output->begin()) = *(input.begin());
    // c[1-m] *= g
    std::transform(++(input.begin()), input.end(), ++(output->begin()),
                   [this](double c) { return c * gamma_; });
    return true;
  }

 private:
  const int num_order_;
  const double gamma_;
  DISALLOW_COPY_AND_ASSIGN(GammaMultiplicationModule);
};

}  // namespace

namespace sptk {

MelGeneralizedCepstrumToMelGeneralizedCepstrum::
    MelGeneralizedCepstrumToMelGeneralizedCepstrum(
        int num_input_order, double input_alpha, double input_gamma,
        bool is_normalized_input, bool is_multiplied_input,
        int num_output_order, double output_alpha, double output_gamma,
        bool is_normalized_output, bool is_multiplied_output)
    : num_input_order_(num_input_order),
      input_alpha_(input_alpha),
      input_gamma_(input_gamma),
      is_normalized_input_(is_normalized_input),
      is_multiplied_input_(is_multiplied_input),
      num_output_order_(num_output_order),
      output_alpha_(output_alpha),
      output_gamma_(output_gamma),
      is_normalized_output_(is_normalized_output),
      is_multiplied_output_(is_multiplied_output),
      alpha_transform_(0.0),
      is_valid_(true) {
  if (num_input_order_ < 0 || !sptk::IsValidAlpha(input_alpha_) ||
      !sptk::IsValidGamma(input_gamma_) || num_output_order_ < 0 ||
      !sptk::IsValidAlpha(output_alpha_) ||
      !sptk::IsValidGamma(output_gamma_) ||
      (is_multiplied_input_ && 0.0 == input_gamma_)) {
    is_valid_ = false;
    return;
  }

  if (input_alpha_ != output_alpha_) {
    alpha_transform_ =
        (output_alpha_ - input_alpha_) / (1.0 - input_alpha_ * output_alpha_);
  }

  // ch_a: change alpha
  // norm_i: normalized input
  // norm_o: normalized output
  // ch_g: change gamma
  // ch_m: change order

  // ch_a=F norm_i=F norm_o=F ch_g=F ch_m=F
  //          ->       ->       ->       ->
  // ch_a=F norm_i=F norm_o=F ch_g=F ch_m=T
  //          ->       -> gnorm -> gc2gc -> ignorm
  // ch_a=F norm_i=F norm_o=F ch_g=T ch_m=F
  //          ->       -> gnorm -> gc2gc -> ignorm
  // ch_a=F norm_i=F norm_o=F ch_g=T ch_m=T
  //          ->       -> gnorm -> gc2gc -> ignorm

  // ch_a=F norm_i=F norm_o=T ch_g=F ch_m=F
  //          ->       -> gnorm ->       ->
  // ch_a=F norm_i=F norm_o=T ch_g=F ch_m=T
  //          ->       -> gnorm -> gc2gc ->
  // ch_a=F norm_i=F norm_o=T ch_g=T ch_m=F
  //          ->       -> gnorm -> gc2gc ->
  // ch_a=F norm_i=F norm_o=T ch_g=T ch_m=T
  //          ->       -> gnorm -> gc2gc ->

  // ch_a=F norm_i=T norm_o=F ch_g=F ch_m=F
  //          ->       ->       ->       -> ignorm
  // ch_a=F norm_i=T norm_o=F ch_g=F ch_m=T
  //          ->       ->       -> gc2gc -> ignorm
  // ch_a=F norm_i=T norm_o=F ch_g=T ch_m=F
  //          ->       ->       -> gc2gc -> ignorm
  // ch_a=F norm_i=T norm_o=F ch_g=T ch_m=T
  //          ->       ->       -> gc2gc -> ignorm

  // ch_a=F norm_i=T norm_o=T ch_g=F ch_m=F
  //          ->       ->       ->       ->
  // ch_a=F norm_i=T norm_o=T ch_g=F ch_m=T
  //          ->       ->       -> gc2gc ->
  // ch_a=F norm_i=T norm_o=T ch_g=T ch_m=F
  //          ->       ->       -> gc2gc ->
  // ch_a=F norm_i=T norm_o=T ch_g=T ch_m=T
  //          ->       ->       -> gc2gc ->

  // ch_a=T norm_i=F norm_o=F ch_g=F ch_m=F
  //          -> freqt ->       ->       ->
  // ch_a=T norm_i=F norm_o=F ch_g=F ch_m=T
  //          -> freqt ->       ->       ->
  // ch_a=T norm_i=F norm_o=F ch_g=T ch_m=F
  //          -> freqt -> gnorm -> gc2gc -> ignorm
  // ch_a=T norm_i=F norm_o=F ch_g=T ch_m=T
  //          -> freqt -> gnorm -> gc2gc -> ignorm

  // ch_a=T norm_i=F norm_o=T ch_g=F ch_m=F
  //          -> freqt -> gnorm ->       ->
  // ch_a=T norm_i=F norm_o=T ch_g=F ch_m=T
  //          -> freqt -> gnorm ->       ->
  // ch_a=T norm_i=F norm_o=T ch_g=T ch_m=F
  //          -> freqt -> gnorm -> gc2gc ->
  // ch_a=T norm_i=F norm_o=T ch_g=T ch_m=T
  //          -> freqt -> gnorm -> gc2gc ->

  // ch_a=T norm_i=T norm_o=F ch_g=F ch_m=F
  //   ignorm -> freqt ->       ->       ->
  // ch_a=T norm_i=T norm_o=F ch_g=F ch_m=T
  //   ignorm -> freqt ->       ->       ->
  // ch_a=T norm_i=T norm_o=F ch_g=T ch_m=F
  //   ignorm -> freqt -> gnorm -> gc2gc -> ignorm
  // ch_a=T norm_i=T norm_o=F ch_g=T ch_m=T
  //   ignorm -> freqt -> gnorm -> gc2gc -> ignorm

  // ch_a=T norm_i=T norm_o=T ch_g=F ch_m=F
  //   ignorm -> freqt -> gnorm ->       ->
  // ch_a=T norm_i=T norm_o=T ch_g=F ch_m=T
  //   ignorm -> freqt -> gnorm ->       ->
  // ch_a=T norm_i=T norm_o=T ch_g=T ch_m=F
  //   ignorm -> freqt -> gnorm -> gc2gc ->
  // ch_a=T norm_i=T norm_o=T ch_g=T ch_m=T
  //   ignorm -> freqt -> gnorm -> gc2gc ->

  if (0.0 == alpha_transform_) {
    if (num_input_order_ == num_output_order_ &&
        input_gamma_ == output_gamma_) {
      if (!is_multiplied_input_ && is_multiplied_output_)
        modules_.push_back(
            new GammaMultiplicationModule(num_input_order_, input_gamma_));
      if (!is_normalized_input_ && is_normalized_output_)
        modules_.push_back(
            new GainNormalizationModule(num_input_order_, input_gamma_));
      if (is_normalized_input_ && !is_normalized_output_)
        modules_.push_back(new InverseGainNormalizationModule(num_output_order_,
                                                              output_gamma_));
      if (is_multiplied_input_ && !is_multiplied_output_)
        modules_.push_back(
            new GammaDivisionModule(num_output_order_, output_gamma_));
    } else {
      if (is_multiplied_input_)
        modules_.push_back(
            new GammaDivisionModule(num_input_order_, input_gamma_));
      if (!is_normalized_input_)
        modules_.push_back(
            new GainNormalizationModule(num_input_order_, input_gamma_));
      modules_.push_back(
          new MelGeneralizedCepstrumToMelGeneralizedCepstrumModule(
              num_input_order_, num_output_order_, input_gamma_,
              output_gamma_));
      if (!is_normalized_output_)
        modules_.push_back(new InverseGainNormalizationModule(num_output_order_,
                                                              output_gamma_));
      if (is_multiplied_output_)
        modules_.push_back(
            new GammaMultiplicationModule(num_output_order_, output_gamma_));
    }
  } else {
    if (is_multiplied_input_)
      modules_.push_back(
          new GammaDivisionModule(num_input_order_, input_gamma_));
    if (is_normalized_input_)
      modules_.push_back(
          new InverseGainNormalizationModule(num_input_order_, input_gamma_));
    modules_.push_back(new FrequencyTransformModule(
        num_input_order_, num_output_order_, alpha_transform_));
    if (is_normalized_output_ || input_gamma_ != output_gamma_)
      modules_.push_back(
          new GainNormalizationModule(num_output_order_, input_gamma_));
    if (input_gamma_ != output_gamma_)
      modules_.push_back(
          new MelGeneralizedCepstrumToMelGeneralizedCepstrumModule(
              num_output_order_, num_output_order_, input_gamma_,
              output_gamma_));
    if (!is_normalized_output_ && input_gamma_ != output_gamma_)
      modules_.push_back(
          new InverseGainNormalizationModule(num_output_order_, output_gamma_));
    if (is_multiplied_output_)
      modules_.push_back(
          new GammaMultiplicationModule(num_output_order_, output_gamma_));
  }

  for (std::vector<MelGeneralizedCepstrumToMelGeneralizedCepstrum::
                       ModuleInterface*>::iterator itr(modules_.begin());
       itr != modules_.end(); ++itr) {
    if (!(*itr)->IsValid()) {
      is_valid_ = false;
      break;
    }
  }
}

bool MelGeneralizedCepstrumToMelGeneralizedCepstrum::Run(
    const std::vector<double>& input, std::vector<double>* output,
    MelGeneralizedCepstrumToMelGeneralizedCepstrum::Buffer* buffer) const {
  if (!is_valid_ ||
      input.size() != static_cast<std::size_t>(num_input_order_ + 1) ||
      NULL == output || NULL == buffer) {
    return false;
  }

  if (modules_.empty()) {
    if (output->size() != static_cast<std::size_t>(num_output_order_ + 1)) {
      output->resize(num_output_order_ + 1);
    }
    std::copy(input.begin(), input.end(), output->begin());
    return true;
  }

  for (std::vector<MelGeneralizedCepstrumToMelGeneralizedCepstrum::
                       ModuleInterface*>::const_iterator itr(modules_.begin());
       itr != modules_.end(); ++itr) {
    if (itr != modules_.begin()) {
      output->swap(buffer->temporary_mel_generalized_cepstrum_);
    }
    if (!(*itr)->Run((itr == modules_.begin())
                         ? input
                         : buffer->temporary_mel_generalized_cepstrum_,
                     output, &(buffer->frequency_transform_buffer_))) {
      return false;
    }
  }

  return true;
}

}  // namespace sptk
