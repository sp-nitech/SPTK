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

#include "SPTK/math/fourier_transform.h"

#include "SPTK/math/discrete_fourier_transform.h"
#include "SPTK/math/fast_fourier_transform.h"

namespace {

class FastFourierTransformWrapper
    : public sptk::FourierTransform::FourierTransformInterface {
 public:
  explicit FastFourierTransformWrapper(int fft_length)
      : fast_fourier_tranform_(fft_length) {
  }

  virtual ~FastFourierTransformWrapper() {
  }

  virtual int GetLength() const {
    return fast_fourier_tranform_.GetFftLength();
  }

  virtual bool IsValid() const {
    return fast_fourier_tranform_.IsValid();
  }

  virtual bool Run(const std::vector<double>& real_part_input,
                   const std::vector<double>& imag_part_input,
                   std::vector<double>* real_part_output,
                   std::vector<double>* imag_part_output) const {
    return fast_fourier_tranform_.Run(real_part_input, imag_part_input,
                                      real_part_output, imag_part_output);
  }

  virtual bool Run(std::vector<double>* real_part,
                   std::vector<double>* imag_part) const {
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

  virtual ~DiscreteFourierTransformWrapper() {
  }

  virtual int GetLength() const {
    return discrete_fourier_tranform_.GetDftLength();
  }

  virtual bool IsValid() const {
    return discrete_fourier_tranform_.IsValid();
  }

  virtual bool Run(const std::vector<double>& real_part_input,
                   const std::vector<double>& imag_part_input,
                   std::vector<double>* real_part_output,
                   std::vector<double>* imag_part_output) const {
    return discrete_fourier_tranform_.Run(real_part_input, imag_part_input,
                                          real_part_output, imag_part_output);
  }

  virtual bool Run(std::vector<double>* real_part,
                   std::vector<double>* imag_part) const {
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
