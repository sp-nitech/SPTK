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

#ifndef SPTK_MATH_INVERSE_FAST_FOURIER_TRANSFORM_FOR_REAL_SEQUENCE_H_
#define SPTK_MATH_INVERSE_FAST_FOURIER_TRANSFORM_FOR_REAL_SEQUENCE_H_

#include <vector>  // std::vector

#include "SPTK/math/fast_fourier_transform_for_real_sequence.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

class InverseFastFourierTransformForRealSequence {
 public:
  class Buffer {
   public:
    Buffer() {
    }
    virtual ~Buffer() {
    }

   private:
    FastFourierTransformForRealSequence::Buffer fast_fourier_transform_buffer_;
    friend class InverseFastFourierTransformForRealSequence;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  //
  InverseFastFourierTransformForRealSequence(int num_order, int fft_size);

  //
  virtual ~InverseFastFourierTransformForRealSequence() {
  }

  //
  int GetNumOrder() const {
    return fast_fourier_transform_.GetNumOrder();
  }

  //
  int GetFftSize() const {
    return fast_fourier_transform_.GetFftSize();
  }

  //
  bool IsValid() const {
    return fast_fourier_transform_.IsValid();
  }

  //
  bool Run(const std::vector<double>& real_part_input,
           std::vector<double>* real_part_output,
           std::vector<double>* imaginary_part_output,
           InverseFastFourierTransformForRealSequence::Buffer* buffer) const;

 private:
  //
  const FastFourierTransformForRealSequence fast_fourier_transform_;

  //
  DISALLOW_COPY_AND_ASSIGN(InverseFastFourierTransformForRealSequence);
};

}  // namespace sptk

#endif  // SPTK_MATH_INVERSE_FAST_FOURIER_TRANSFORM_FOR_REAL_SEQUENCE_H_
