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
//                1996-2020  Nagoya Institute of Technology          //
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

#ifndef SPTK_MATH_TWO_DIMENSIONAL_INVERSE_FAST_FOURIER_TRANSFORM_H_
#define SPTK_MATH_TWO_DIMENSIONAL_INVERSE_FAST_FOURIER_TRANSFORM_H_

#include <vector>  // std::vector

#include "SPTK/math/inverse_fast_fourier_transform.h"
#include "SPTK/math/matrix.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Calculate 2D inverse DFT of complex-valued input data.
 *
 * The inputs are two @f$M \times N@f$ matrices that represent real and
 * imaginary parts:
 * @f[
 *   \begin{array}{cc}
 *     \mathrm{Re}(\boldsymbol{X}), & \mathrm{Im}(\boldsymbol{X}).
 *   \end{array}
 * @f]
 * The outputs are two @f$L \times L @f$ matrices:
 * @f[
 *   \begin{array}{cccc}
 *     \mathrm{Re}(\boldsymbol{x}), & \mathrm{Im}(\boldsymbol{x}),
 *   \end{array}
 * @f]
 * where @f$L@f$ is the FFT length and must be a power of two.
 */
class TwoDimensionalInverseFastFourierTransform {
 public:
  /**
   * Buffer for TwoDimensionalInverseFastFourierTransform class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    std::vector<double> real_part_input_;
    std::vector<double> imag_part_input_;
    std::vector<std::vector<double> > first_real_part_outputs_;
    std::vector<std::vector<double> > first_imag_part_outputs_;
    std::vector<std::vector<double> > second_real_part_outputs_;
    std::vector<std::vector<double> > second_imag_part_outputs_;

    friend class TwoDimensionalInverseFastFourierTransform;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_row Number of rows, @f$M@f$.
   * @param[in] num_column Number of columns @f$N@f$.
   * @param[in] fft_length FFT length, @f$L@f$.
   */
  TwoDimensionalInverseFastFourierTransform(int num_row, int num_column,
                                            int fft_length);

  virtual ~TwoDimensionalInverseFastFourierTransform() {
  }

  /**
   * @return Number of rows of input.
   */
  int GetNumRow() const {
    return num_row_;
  }

  /**
   * @return Number of columns of input.
   */
  int GetNumColumn() const {
    return num_column_;
  }

  /**
   * @return FFT length.
   */
  int GetFftLength() const {
    return fft_length_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] real_part_input Real part of input.
   * @param[in] imag_part_input Imaginary part of input.
   * @param[out] real_part_output Real part of output.
   * @param[out] imag_part_output Imaginary part of output.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const sptk::Matrix& real_part_input,
           const sptk::Matrix& imag_part_input, sptk::Matrix* real_part_output,
           sptk::Matrix* imag_part_output,
           TwoDimensionalInverseFastFourierTransform::Buffer* buffer) const;

  /**
   * @param[in,out] real_part Real part.
   * @param[in,out] imag_part Imaginary part.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(sptk::Matrix* real_part, sptk::Matrix* imag_part,
           TwoDimensionalInverseFastFourierTransform::Buffer* buffer) const;

 private:
  const int num_row_;
  const int num_column_;
  const int fft_length_;

  const InverseFastFourierTransform inverse_fast_fourier_transform_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(TwoDimensionalInverseFastFourierTransform);
};

}  // namespace sptk

#endif  // SPTK_MATH_TWO_DIMENSIONAL_INVERSE_FAST_FOURIER_TRANSFORM_H_
