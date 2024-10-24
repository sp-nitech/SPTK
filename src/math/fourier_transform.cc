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

#include "SPTK/math/fourier_transform.h"

#include <vector>  // std::vector

#include "SPTK/math/discrete_fourier_transform.h"
#include "SPTK/math/fast_fourier_transform.h"

namespace {

class FastFourierTransformWrapper
    : public sptk::FourierTransform::FourierTransformInterface {
 public:
  explicit FastFourierTransformWrapper(int fft_length)
      : fast_fourier_tranform_(fft_length) {
  }

  ~FastFourierTransformWrapper() override {
  }

  int GetLength() const override {
    return fast_fourier_tranform_.GetFftLength();
  }

  bool IsValid() const override {
    return fast_fourier_tranform_.IsValid();
  }

  bool Run(const std::vector<double>& real_part_input,
           const std::vector<double>& imag_part_input,
           std::vector<double>* real_part_output,
           std::vector<double>* imag_part_output) const override {
    return fast_fourier_tranform_.Run(real_part_input, imag_part_input,
                                      real_part_output, imag_part_output);
  }

  bool Run(std::vector<double>* real_part,
           std::vector<double>* imag_part) const override {
    return fast_fourier_tranform_.Run(real_part, imag_part);
  }

 private:
  const sptk::FastFourierTransform fast_fourier_tranform_;

  DISALLOW_COPY_AND_ASSIGN(FastFourierTransformWrapper);
};

class DiscreteFourierTransformWrapper
    : public sptk::FourierTransform::FourierTransformInterface {
 public:
  explicit DiscreteFourierTransformWrapper(int dft_length)
      : discrete_fourier_tranform_(dft_length) {
  }

  ~DiscreteFourierTransformWrapper() override {
  }

  int GetLength() const override {
    return discrete_fourier_tranform_.GetDftLength();
  }

  bool IsValid() const override {
    return discrete_fourier_tranform_.IsValid();
  }

  bool Run(const std::vector<double>& real_part_input,
           const std::vector<double>& imag_part_input,
           std::vector<double>* real_part_output,
           std::vector<double>* imag_part_output) const override {
    return discrete_fourier_tranform_.Run(real_part_input, imag_part_input,
                                          real_part_output, imag_part_output);
  }

  bool Run(std::vector<double>* real_part,
           std::vector<double>* imag_part) const override {
    return discrete_fourier_tranform_.Run(real_part, imag_part);
  }

 private:
  const sptk::DiscreteFourierTransform discrete_fourier_tranform_;

  DISALLOW_COPY_AND_ASSIGN(DiscreteFourierTransformWrapper);
};

}  // namespace

namespace sptk {

FourierTransform::FourierTransform(int length) {
  if (sptk::IsPowerOfTwo(length)) {
    fourier_transform_ = new FastFourierTransformWrapper(length);
  } else {
    fourier_transform_ = new DiscreteFourierTransformWrapper(length);
  }
}

}  // namespace sptk
