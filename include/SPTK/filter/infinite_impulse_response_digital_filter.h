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
