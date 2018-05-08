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

#ifndef SPTK_MATH_TWO_DIMENSIONAL_FAST_FOURIER_TRANSFORM_H_
#define SPTK_MATH_TWO_DIMENSIONAL_FAST_FOURIER_TRANSFORM_H_

#include <vector>  // std::vector

#include "SPTK/math/fast_fourier_transform.h"
#include "SPTK/math/matrix.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

class TwoDimensionalFastFourierTransform {
 public:
  class Buffer {
   public:
    Buffer() {
    }
    virtual ~Buffer() {
    }

   private:
    std::vector<double> real_part_input_;
    std::vector<double> imaginary_part_input_;
    std::vector<std::vector<double> > first_real_part_outputs_;
    std::vector<std::vector<double> > first_imaginary_part_outputs_;
    std::vector<std::vector<double> > second_real_part_outputs_;
    std::vector<std::vector<double> > second_imaginary_part_outputs_;
    friend class TwoDimensionalFastFourierTransform;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  //
  TwoDimensionalFastFourierTransform(int num_row, int num_column,
                                     int fft_length);

  //
  virtual ~TwoDimensionalFastFourierTransform() {
  }

  //
  int GetNumRow() const {
    return num_row_;
  }

  //
  int GetNumColumn() const {
    return num_column_;
  }

  //
  int GetFftLength() const {
    return fft_length_;
  }

  //
  bool IsValid() const {
    return is_valid_;
  }

  //
  bool Run(const sptk::Matrix& real_part_input,
           const sptk::Matrix& imaginary_part_input,
           sptk::Matrix* real_part_output, sptk::Matrix* imaginary_part_output,
           TwoDimensionalFastFourierTransform::Buffer* buffer) const;

 private:
  //
  const int num_row_;

  //
  const int num_column_;

  //
  const int fft_length_;

  //
  const FastFourierTransform fast_fourier_transform_;

  //
  bool is_valid_;

  //
  DISALLOW_COPY_AND_ASSIGN(TwoDimensionalFastFourierTransform);
};

}  // namespace sptk

#endif  // SPTK_MATH_TWO_DIMENSIONAL_FAST_FOURIER_TRANSFORM_H_
