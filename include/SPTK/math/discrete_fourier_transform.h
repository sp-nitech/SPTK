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

#ifndef SPTK_MATH_DISCRETE_FOURIER_TRANSFORM_H_
#define SPTK_MATH_DISCRETE_FOURIER_TRANSFORM_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Calculate DFT of complex-valued input data.
 *
 * The inputs are @f$L@f$-length complex-valued data:
 * @f[
 *   \begin{array}{cccc}
 *   \mathrm{Re}(x(0)), & \mathrm{Re}(x(1)), & \ldots, & \mathrm{Re}(x(L-1)), \\
 *   \mathrm{Im}(x(0)), & \mathrm{Im}(x(1)), & \ldots, & \mathrm{Im}(x(L-1)).
 *   \end{array}
 * @f]
 * The outputs are
 * @f[
 *   \begin{array}{cccc}
 *   \mathrm{Re}(X(0)), & \mathrm{Re}(X(1)), & \ldots, & \mathrm{Re}(X(L-1)), \\
 *   \mathrm{Im}(X(0)), & \mathrm{Im}(X(1)), & \ldots, & \mathrm{Im}(X(L-1)).
 *   \end{array}
 * @f]
 * They are computed as
 * @f[
 *   X(k) = \sum_{n=0}^{L-1} x(n) e^{-2\pi nk / L}.
 * @f]
 */
class DiscreteFourierTransform {
 public:
  /**
   * @param[in] dft_length DFT length, @f$L@f$.
   */
  explicit DiscreteFourierTransform(int dft_length);

  virtual ~DiscreteFourierTransform() {
  }

  /**
   * @return DFT length.
   */
  int GetDftLength() const {
    return dft_length_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] real_part_input @f$L@f$-length real part of input.
   * @param[in] imag_part_input @f$L@f$-length imaginary part of input.
   * @param[out] real_part_output @f$L@f$-length real part of output.
   * @param[out] imag_part_output @f$L@f$-length imaginary part of output.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& real_part_input,
           const std::vector<double>& imag_part_input,
           std::vector<double>* real_part_output,
           std::vector<double>* imag_part_output) const;

  /**
   * @param[in,out] real_part @f$L@f$-length real part.
   * @param[in,out] imag_part @f$L@f$-length imaginary part.
   * @return True on success, false on failure.
   */
  bool Run(std::vector<double>* real_part,
           std::vector<double>* imag_part) const;

 private:
  const int dft_length_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(DiscreteFourierTransform);
};

}  // namespace sptk

#endif  // SPTK_MATH_DISCRETE_FOURIER_TRANSFORM_H_
