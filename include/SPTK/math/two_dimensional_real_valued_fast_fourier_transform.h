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

#ifndef SPTK_MATH_TWO_DIMENSIONAL_REAL_VALUED_FAST_FOURIER_TRANSFORM_H_
#define SPTK_MATH_TWO_DIMENSIONAL_REAL_VALUED_FAST_FOURIER_TRANSFORM_H_

#include <vector>  // std::vector

#include "SPTK/math/fast_fourier_transform.h"
#include "SPTK/math/matrix.h"
#include "SPTK/math/real_valued_fast_fourier_transform.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Calculate 2D DFT of real-valued input data.
 *
 * The input is a real-valued @f$M \times N@f$ matrix:
 * @f[
 *   \begin{array}{cc}
 *     \boldsymbol{x}.
 *   \end{array}
 * @f]
 * The outputs are two @f$L \times L @f$ matrices:
 * @f[
 *   \begin{array}{cccc}
 *     \mathrm{Re}(\boldsymbol{X}), & \mathrm{Im}(\boldsymbol{X}),
 *   \end{array}
 * @f]
 * where @f$L@f$ is the FFT length and must be a power of two.
 */
class TwoDimensionalRealValuedFastFourierTransform {
 public:
  /**
   * Buffer for TwoDimensionalRealValuedFastFourierTransform class.
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
    RealValuedFastFourierTransform::Buffer fast_fourier_transform_buffer_;

    friend class TwoDimensionalRealValuedFastFourierTransform;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_row Number of rows, @f$M@f$.
   * @param[in] num_column Number of columns, @f$N@f$.
   * @param[in] fft_length FFT length, @f$L@f$.
   */
  TwoDimensionalRealValuedFastFourierTransform(int num_row, int num_column,
                                               int fft_length);

  virtual ~TwoDimensionalRealValuedFastFourierTransform() {
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
   * @param[out] real_part_output Real part of output.
   * @param[out] imag_part_output Imaginary part of output.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const Matrix& real_part_input, Matrix* real_part_output,
           Matrix* imag_part_output,
           TwoDimensionalRealValuedFastFourierTransform::Buffer* buffer) const;

  /**
   * @param[in,out] real_part Real part.
   * @param[out] imag_part Imaginary part.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(Matrix* real_part, Matrix* imag_part,
           TwoDimensionalRealValuedFastFourierTransform::Buffer* buffer) const;

 private:
  const int num_row_;
  const int num_column_;
  const int fft_length_;

  const FastFourierTransform fast_fourier_transform_;
  const RealValuedFastFourierTransform real_valued_fast_fourier_transform_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(TwoDimensionalRealValuedFastFourierTransform);
};

}  // namespace sptk

#endif  // SPTK_MATH_TWO_DIMENSIONAL_REAL_VALUED_FAST_FOURIER_TRANSFORM_H_
