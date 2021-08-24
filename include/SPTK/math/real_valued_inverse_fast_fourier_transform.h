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

#ifndef SPTK_MATH_REAL_VALUED_INVERSE_FAST_FOURIER_TRANSFORM_H_
#define SPTK_MATH_REAL_VALUED_INVERSE_FAST_FOURIER_TRANSFORM_H_

#include <vector>  // std::vector

#include "SPTK/math/real_valued_fast_fourier_transform.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Calculate inverse DFT of real-valued input data.
 *
 * This is almost similar to RealValuedFastFourierTransform. The DFT results
 * are divided by the FFT length @f$L@f$.
 */
class RealValuedInverseFastFourierTransform {
 public:
  /**
   * Buffer for RealValuedInverseFastFourierTransform class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    RealValuedFastFourierTransform::Buffer fast_fourier_transform_buffer_;

    friend class RealValuedInverseFastFourierTransform;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] fft_length FFT length, @f$L@f$.
   */
  explicit RealValuedInverseFastFourierTransform(int fft_length);

  /**
   * @param[in] num_order Order of input, @f$M@f$.
   * @param[in] fft_length FFT length, @f$L@f$.
   */
  RealValuedInverseFastFourierTransform(int num_order, int fft_length);

  virtual ~RealValuedInverseFastFourierTransform() {
  }

  /**
   * @return Order of input.
   */
  int GetNumOrder() const {
    return fast_fourier_transform_.GetNumOrder();
  }

  /**
   * @return FFT length.
   */
  int GetFftLength() const {
    return fast_fourier_transform_.GetFftLength();
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return fast_fourier_transform_.IsValid();
  }

  /**
   * @param[in] real_part_input @f$M@f$-th order real part of input.
   * @param[out] real_part_output @f$L@f$-length real part of output.
   * @param[out] imag_part_output @f$L@f$-length imaginary part of output.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& real_part_input,
           std::vector<double>* real_part_output,
           std::vector<double>* imag_part_output,
           RealValuedInverseFastFourierTransform::Buffer* buffer) const;

  /**
   * @param[in,out] real_part Real part.
   * @param[out] imag_part Imaginary part.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(std::vector<double>* real_part, std::vector<double>* imag_part,
           RealValuedInverseFastFourierTransform::Buffer* buffer) const;

 private:
  const RealValuedFastFourierTransform fast_fourier_transform_;

  DISALLOW_COPY_AND_ASSIGN(RealValuedInverseFastFourierTransform);
};

}  // namespace sptk

#endif  // SPTK_MATH_REAL_VALUED_INVERSE_FAST_FOURIER_TRANSFORM_H_
