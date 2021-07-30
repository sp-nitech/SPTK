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

#ifndef SPTK_FILTER_INFINITE_IMPULSE_RESPONSE_DIGITAL_FILTER_H_
#define SPTK_FILTER_INFINITE_IMPULSE_RESPONSE_DIGITAL_FILTER_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Apply infinite impulse response digital filter in time domain.
 *
 * Input signal is filtered by
 * @f[
 *   H(z) = K \frac{\displaystyle\sum_{m=0}^M b(m) z^{-m}}
 *                 {1 + \displaystyle\sum_{n=1}^N a(n) z^{-n}},
 * @f]
 * where @f$K@f$ is the gain.
 */
class InfiniteImpulseResponseDigitalFilter {
 public:
  /**
   * Buffer for InfiniteImpulseResponseDigitalFilter class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    std::vector<double> d_;
    int p_;

    friend class InfiniteImpulseResponseDigitalFilter;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] denominator_coefficients Denominator coefficients,
   *            @f$K@f$ and @f$\{ a(n) \}_{n=1}^N@f$.
   * @param[in] numerator_coefficients Numerator coefficients,
   *            @f$\{ b(m) \}_{m=0}^M@f$.
   */
  InfiniteImpulseResponseDigitalFilter(
      const std::vector<double>& denominator_coefficients,
      const std::vector<double>& numerator_coefficients);

  virtual ~InfiniteImpulseResponseDigitalFilter() {
  }

  /**
   * @return Order of denominator coefficients.
   */
  int GetNumDenominatorOrder() const {
    return num_denominator_order_;
  }

  /**
   * @return Order of numerator coefficients.
   */
  int GetNumNumeratorOrder() const {
    return num_numerator_order_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] input Filter input.
   * @param[out] output Filter output.
   * @param[in,out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(double input, double* output,
           InfiniteImpulseResponseDigitalFilter::Buffer* buffer) const;

  /**
   * @param[in,out] input_and_output Input/output signal.
   * @param[in,out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(double* input_and_output,
           InfiniteImpulseResponseDigitalFilter::Buffer* buffer) const;

 private:
  const std::vector<double> denominator_coefficients_;
  const std::vector<double> numerator_coefficients_;
  const int num_denominator_order_;
  const int num_numerator_order_;
  const int num_filter_order_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(InfiniteImpulseResponseDigitalFilter);
};

}  // namespace sptk

#endif  // SPTK_FILTER_INFINITE_IMPULSE_RESPONSE_DIGITAL_FILTER_H_
