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

#include "SPTK/conversion/spectrum_to_spectrum.h"

#include <algorithm>  // std::copy, std::max, std::max_element, std::transform
#include <cfloat>     // DBL_MAX
#include <cmath>      // std::exp, std::log, std::log10, std::pow, std::sqrt
#include <cstddef>    // std::size_t

namespace {

class LogAmplitudeSpectrumInDecibelsToLogAmplitudeSpectrum
    : public sptk::SpectrumToSpectrum::OperationInterface {
 public:
  LogAmplitudeSpectrumInDecibelsToLogAmplitudeSpectrum() {
  }
  virtual bool Run(std::vector<double>* input_and_output) const {
    std::transform(input_and_output->begin(), input_and_output->end(),
                   input_and_output->begin(),
                   [](double x) { return x / sptk::kNeper; });
    return true;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(
      LogAmplitudeSpectrumInDecibelsToLogAmplitudeSpectrum);
};

class LogAmplitudeSpectrumInDecibelsToAmplitudeSpectrum
    : public sptk::SpectrumToSpectrum::OperationInterface {
 public:
  LogAmplitudeSpectrumInDecibelsToAmplitudeSpectrum() {
  }
  virtual bool Run(std::vector<double>* input_and_output) const {
    std::transform(input_and_output->begin(), input_and_output->end(),
                   input_and_output->begin(),
                   [](double x) { return std::pow(10.0, 0.05 * x); });
    return true;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(LogAmplitudeSpectrumInDecibelsToAmplitudeSpectrum);
};

class LogAmplitudeSpectrumInDecibelsToPowerSpectrum
    : public sptk::SpectrumToSpectrum::OperationInterface {
 public:
  LogAmplitudeSpectrumInDecibelsToPowerSpectrum() {
  }
  virtual bool Run(std::vector<double>* input_and_output) const {
    std::transform(input_and_output->begin(), input_and_output->end(),
                   input_and_output->begin(),
                   [](double x) { return std::pow(10.0, 0.1 * x); });
    return true;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(LogAmplitudeSpectrumInDecibelsToPowerSpectrum);
};

class LogAmplitudeSpectrumToLogAmplitudeSpectrumInDecibels
    : public sptk::SpectrumToSpectrum::OperationInterface {
 public:
  LogAmplitudeSpectrumToLogAmplitudeSpectrumInDecibels() {
  }
  virtual bool Run(std::vector<double>* input_and_output) const {
    std::transform(input_and_output->begin(), input_and_output->end(),
                   input_and_output->begin(),
                   [](double x) { return x * sptk::kNeper; });
    return true;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(
      LogAmplitudeSpectrumToLogAmplitudeSpectrumInDecibels);
};

class LogAmplitudeSpectrumToAmplitudeSpectrum
    : public sptk::SpectrumToSpectrum::OperationInterface {
 public:
  LogAmplitudeSpectrumToAmplitudeSpectrum() {
  }
  virtual bool Run(std::vector<double>* input_and_output) const {
    std::transform(input_and_output->begin(), input_and_output->end(),
                   input_and_output->begin(),
                   [](double x) { return std::exp(x); });
    return true;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(LogAmplitudeSpectrumToAmplitudeSpectrum);
};

class LogAmplitudeSpectrumToPowerSpectrum
    : public sptk::SpectrumToSpectrum::OperationInterface {
 public:
  LogAmplitudeSpectrumToPowerSpectrum() {
  }
  virtual bool Run(std::vector<double>* input_and_output) const {
    std::transform(input_and_output->begin(), input_and_output->end(),
                   input_and_output->begin(),
                   [](double x) { return std::exp(2.0 * x); });
    return true;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(LogAmplitudeSpectrumToPowerSpectrum);
};

class AmplitudeSpectrumToLogAmplitudeSpectrumInDecibels
    : public sptk::SpectrumToSpectrum::OperationInterface {
 public:
  AmplitudeSpectrumToLogAmplitudeSpectrumInDecibels() {
  }
  virtual bool Run(std::vector<double>* input_and_output) const {
    std::transform(input_and_output->begin(), input_and_output->end(),
                   input_and_output->begin(),
                   [](double x) { return 20.0 * std::log10(x); });
    return true;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(AmplitudeSpectrumToLogAmplitudeSpectrumInDecibels);
};

class AmplitudeSpectrumToLogAmplitudeSpectrum
    : public sptk::SpectrumToSpectrum::OperationInterface {
 public:
  AmplitudeSpectrumToLogAmplitudeSpectrum() {
  }
  virtual bool Run(std::vector<double>* input_and_output) const {
    std::transform(input_and_output->begin(), input_and_output->end(),
                   input_and_output->begin(),
                   [](double x) { return std::log(x); });
    return true;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(AmplitudeSpectrumToLogAmplitudeSpectrum);
};

class AmplitudeSpectrumToPowerSpectrum
    : public sptk::SpectrumToSpectrum::OperationInterface {
 public:
  AmplitudeSpectrumToPowerSpectrum() {
  }
  virtual bool Run(std::vector<double>* input_and_output) const {
    std::transform(input_and_output->begin(), input_and_output->end(),
                   input_and_output->begin(), [](double x) { return x * x; });
    return true;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(AmplitudeSpectrumToPowerSpectrum);
};

class PowerSpectrumToLogAmplitudeSpectrumInDecibels
    : public sptk::SpectrumToSpectrum::OperationInterface {
 public:
  PowerSpectrumToLogAmplitudeSpectrumInDecibels() {
  }
  virtual bool Run(std::vector<double>* input_and_output) const {
    std::transform(input_and_output->begin(), input_and_output->end(),
                   input_and_output->begin(),
                   [](double x) { return 10.0 * std::log10(x); });
    return true;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(PowerSpectrumToLogAmplitudeSpectrumInDecibels);
};

class PowerSpectrumToLogAmplitudeSpectrum
    : public sptk::SpectrumToSpectrum::OperationInterface {
 public:
  PowerSpectrumToLogAmplitudeSpectrum() {
  }
  virtual bool Run(std::vector<double>* input_and_output) const {
    std::transform(input_and_output->begin(), input_and_output->end(),
                   input_and_output->begin(),
                   [](double x) { return 0.5 * std::log(x); });
    return true;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(PowerSpectrumToLogAmplitudeSpectrum);
};

class PowerSpectrumToAmplitudeSpectrum
    : public sptk::SpectrumToSpectrum::OperationInterface {
 public:
  PowerSpectrumToAmplitudeSpectrum() {
  }
  virtual bool Run(std::vector<double>* input_and_output) const {
    std::transform(input_and_output->begin(), input_and_output->end(),
                   input_and_output->begin(),
                   [](double x) { return std::sqrt(x); });
    return true;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(PowerSpectrumToAmplitudeSpectrum);
};

class Addition : public sptk::SpectrumToSpectrum::OperationInterface {
 public:
  explicit Addition(double addend) : addend_(addend) {
  }
  virtual bool Run(std::vector<double>* input_and_output) const {
    std::transform(input_and_output->begin(), input_and_output->end(),
                   input_and_output->begin(),
                   [this](double x) { return x + addend_; });
    return true;
  }

 private:
  const double addend_;
  DISALLOW_COPY_AND_ASSIGN(Addition);
};

class FlooringOnLogarithm
    : public sptk::SpectrumToSpectrum::OperationInterface {
 public:
  explicit FlooringOnLogarithm(double relative_floor)
      : relative_floor_(relative_floor) {
  }
  virtual bool Run(std::vector<double>* input_and_output) const {
    const double max_value(
        *std::max_element(input_and_output->begin(), input_and_output->end()));
    const double floor(max_value + relative_floor_);
    std::transform(input_and_output->begin(), input_and_output->end(),
                   input_and_output->begin(),
                   [floor](double x) { return std::max(x, floor); });
    return true;
  }

 private:
  const double relative_floor_;
  DISALLOW_COPY_AND_ASSIGN(FlooringOnLogarithm);
};

class FlooringOnLinear : public sptk::SpectrumToSpectrum::OperationInterface {
 public:
  explicit FlooringOnLinear(double relative_floor)
      : relative_floor_(relative_floor) {
  }
  virtual bool Run(std::vector<double>* input_and_output) const {
    const double max_value(
        *std::max_element(input_and_output->begin(), input_and_output->end()));
    const double floor(max_value * relative_floor_);
    std::transform(input_and_output->begin(), input_and_output->end(),
                   input_and_output->begin(),
                   [floor](double x) { return std::max(x, floor); });
    return true;
  }

 private:
  const double relative_floor_;
  DISALLOW_COPY_AND_ASSIGN(FlooringOnLinear);
};

void SelectOperation(
    sptk::SpectrumToSpectrum::InputOutputFormats input_format,
    sptk::SpectrumToSpectrum::InputOutputFormats output_format,
    std::vector<sptk::SpectrumToSpectrum::OperationInterface*>* operations) {
  switch (input_format) {
    case sptk::SpectrumToSpectrum::kLogAmplitudeSpectrumInDecibels: {
      switch (output_format) {
        case sptk::SpectrumToSpectrum::kLogAmplitudeSpectrumInDecibels: {
          // nothing to do
          break;
        }
        case sptk::SpectrumToSpectrum::kLogAmplitudeSpectrum: {
          operations->push_back(
              new LogAmplitudeSpectrumInDecibelsToLogAmplitudeSpectrum());
          break;
        }
        case sptk::SpectrumToSpectrum::kAmplitudeSpectrum: {
          operations->push_back(
              new LogAmplitudeSpectrumInDecibelsToAmplitudeSpectrum());
          break;
        }
        case sptk::SpectrumToSpectrum::kPowerSpectrum: {
          operations->push_back(
              new LogAmplitudeSpectrumInDecibelsToPowerSpectrum());
          break;
        }
        default: {
          return;
        }
      }
      break;
    }
    case sptk::SpectrumToSpectrum::kLogAmplitudeSpectrum: {
      switch (output_format) {
        case sptk::SpectrumToSpectrum::kLogAmplitudeSpectrumInDecibels: {
          operations->push_back(
              new LogAmplitudeSpectrumToLogAmplitudeSpectrumInDecibels());
          break;
        }
        case sptk::SpectrumToSpectrum::kLogAmplitudeSpectrum: {
          // nothing to do
          break;
        }
        case sptk::SpectrumToSpectrum::kAmplitudeSpectrum: {
          operations->push_back(new LogAmplitudeSpectrumToAmplitudeSpectrum());
          break;
        }
        case sptk::SpectrumToSpectrum::kPowerSpectrum: {
          operations->push_back(new LogAmplitudeSpectrumToPowerSpectrum());
          break;
        }
        default: {
          return;
        }
      }
      break;
    }
    case sptk::SpectrumToSpectrum::kAmplitudeSpectrum: {
      switch (output_format) {
        case sptk::SpectrumToSpectrum::kLogAmplitudeSpectrumInDecibels: {
          operations->push_back(
              new AmplitudeSpectrumToLogAmplitudeSpectrumInDecibels());
          break;
        }
        case sptk::SpectrumToSpectrum::kLogAmplitudeSpectrum: {
          operations->push_back(new AmplitudeSpectrumToLogAmplitudeSpectrum());
          break;
        }
        case sptk::SpectrumToSpectrum::kAmplitudeSpectrum: {
          // nothing to do
          break;
        }
        case sptk::SpectrumToSpectrum::kPowerSpectrum: {
          operations->push_back(new AmplitudeSpectrumToPowerSpectrum());
          break;
        }
        default: {
          return;
        }
      }
      break;
    }
    case sptk::SpectrumToSpectrum::kPowerSpectrum: {
      switch (output_format) {
        case sptk::SpectrumToSpectrum::kLogAmplitudeSpectrumInDecibels: {
          operations->push_back(
              new PowerSpectrumToLogAmplitudeSpectrumInDecibels());
          break;
        }
        case sptk::SpectrumToSpectrum::kLogAmplitudeSpectrum: {
          operations->push_back(new PowerSpectrumToLogAmplitudeSpectrum());
          break;
        }
        case sptk::SpectrumToSpectrum::kAmplitudeSpectrum: {
          operations->push_back(new PowerSpectrumToAmplitudeSpectrum());
          break;
        }
        case sptk::SpectrumToSpectrum::kPowerSpectrum: {
          // nothing to do
          break;
        }
        default: {
          return;
        }
      }
      break;
    }
    default: {
      return;
    }
  }
}

}  // namespace

namespace sptk {

SpectrumToSpectrum::SpectrumToSpectrum(int fft_length,
                                       InputOutputFormats input_format,
                                       InputOutputFormats output_format,
                                       double epsilon,
                                       double relative_floor_in_decibels)
    : fft_length_(fft_length),
      input_format_(input_format),
      output_format_(output_format),
      epsilon_(epsilon),
      relative_floor_in_decibels_(relative_floor_in_decibels),
      is_valid_(true) {
  if (fft_length_ <= 0 || !IsPowerOfTwo(fft_length_) || input_format_ < 0 ||
      kNumInputOutputFormats <= input_format_ || output_format_ < 0 ||
      kNumInputOutputFormats <= output_format_ || epsilon_ < 0.0 ||
      0.0 <= relative_floor_in_decibels_) {
    is_valid_ = false;
    return;
  }

  if (0.0 != epsilon_) {
    SelectOperation(input_format_, kPowerSpectrum, &operations_);
    operations_.push_back(new Addition(epsilon_));
    SelectOperation(kPowerSpectrum, output_format_, &operations_);
  } else {
    SelectOperation(input_format_, output_format_, &operations_);
  }

  if (-DBL_MAX != relative_floor_in_decibels_) {
    switch (output_format_) {
      case kLogAmplitudeSpectrumInDecibels: {
        operations_.push_back(
            new FlooringOnLogarithm(relative_floor_in_decibels_));
        break;
      }
      case kLogAmplitudeSpectrum: {
        operations_.push_back(new FlooringOnLogarithm(
            relative_floor_in_decibels_ / sptk::kNeper));
        break;
      }
      case kAmplitudeSpectrum: {
        operations_.push_back(new FlooringOnLinear(
            std::pow(10.0, 0.05 * relative_floor_in_decibels_)));
        break;
      }
      case kPowerSpectrum: {
        operations_.push_back(new FlooringOnLinear(
            std::pow(10.0, 0.1 * relative_floor_in_decibels_)));
        break;
      }
      default: {
        return;
      }
    }
  }
}

SpectrumToSpectrum::~SpectrumToSpectrum() {
  for (std::vector<SpectrumToSpectrum::OperationInterface*>::iterator itr(
           operations_.begin());
       itr != operations_.end(); ++itr) {
    delete (*itr);
  }
}

bool SpectrumToSpectrum::Run(const std::vector<double>& input,
                             std::vector<double>* output) const {
  // Check inputs.
  const int length(fft_length_ / 2 + 1);
  if (!is_valid_ || input.size() != static_cast<std::size_t>(length) ||
      NULL == output) {
    return false;
  }

  // Prepare memories.
  if (output->size() != static_cast<std::size_t>(length)) {
    output->resize(length);
  }

  if (&input != output) {
    std::copy(input.begin(), input.end(), output->begin());
  }

  for (std::vector<SpectrumToSpectrum::OperationInterface*>::const_iterator itr(
           operations_.begin());
       itr != operations_.end(); ++itr) {
    if (!(*itr)->Run(output)) {
      return false;
    }
  }

  return true;
}

bool SpectrumToSpectrum::Run(std::vector<double>* input_and_output) const {
  if (NULL == input_and_output) return false;
  return Run(*input_and_output, input_and_output);
}

}  // namespace sptk
