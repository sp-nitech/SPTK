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

#include "SPTK/math/inverse_fourier_transform.h"

#include <vector>  // std::vector

#include "SPTK/math/inverse_discrete_fourier_transform.h"
#include "SPTK/math/inverse_fast_fourier_transform.h"

namespace {

class InverseFastFourierTransformWrapper
    : public sptk::FourierTransform::FourierTransformInterface {
 public:
  explicit InverseFastFourierTransformWrapper(int fft_length)
      : inverse_fast_fourier_tranform_(fft_length) {
  }

  ~InverseFastFourierTransformWrapper() override {
  }

  int GetLength() const override {
    return inverse_fast_fourier_tranform_.GetFftLength();
  }

  bool IsValid() const override {
    return inverse_fast_fourier_tranform_.IsValid();
  }

  bool Run(const std::vector<double>& real_part_input,
           const std::vector<double>& imag_part_input,
           std::vector<double>* real_part_output,
           std::vector<double>* imag_part_output) const override {
    return inverse_fast_fourier_tranform_.Run(
        real_part_input, imag_part_input, real_part_output, imag_part_output);
  }

  bool Run(std::vector<double>* real_part,
           std::vector<double>* imag_part) const override {
    return inverse_fast_fourier_tranform_.Run(real_part, imag_part);
  }

 private:
  const sptk::InverseFastFourierTransform inverse_fast_fourier_tranform_;

  DISALLOW_COPY_AND_ASSIGN(InverseFastFourierTransformWrapper);
};

class InverseDiscreteFourierTransformWrapper
    : public sptk::FourierTransform::FourierTransformInterface {
 public:
  explicit InverseDiscreteFourierTransformWrapper(int dft_length)
      : inverse_discrete_fourier_tranform_(dft_length) {
  }

  ~InverseDiscreteFourierTransformWrapper() override {
  }

  int GetLength() const override {
    return inverse_discrete_fourier_tranform_.GetDftLength();
  }

  bool IsValid() const override {
    return inverse_discrete_fourier_tranform_.IsValid();
  }

  bool Run(const std::vector<double>& real_part_input,
           const std::vector<double>& imag_part_input,
           std::vector<double>* real_part_output,
           std::vector<double>* imag_part_output) const override {
    return inverse_discrete_fourier_tranform_.Run(
        real_part_input, imag_part_input, real_part_output, imag_part_output);
  }

  bool Run(std::vector<double>* real_part,
           std::vector<double>* imag_part) const override {
    return inverse_discrete_fourier_tranform_.Run(real_part, imag_part);
  }

 private:
  const sptk::InverseDiscreteFourierTransform
      inverse_discrete_fourier_tranform_;

  DISALLOW_COPY_AND_ASSIGN(InverseDiscreteFourierTransformWrapper);
};

}  // namespace

namespace sptk {

InverseFourierTransform::InverseFourierTransform(int length) {
  if (sptk::IsPowerOfTwo(length)) {
    inverse_fourier_transform_ = new InverseFastFourierTransformWrapper(length);
  } else {
    inverse_fourier_transform_ =
        new InverseDiscreteFourierTransformWrapper(length);
  }
}

}  // namespace sptk
