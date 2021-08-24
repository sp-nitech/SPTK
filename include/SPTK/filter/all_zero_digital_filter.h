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

#ifndef SPTK_FILTER_ALL_ZERO_DIGITAL_FILTER_H_
#define SPTK_FILTER_ALL_ZERO_DIGITAL_FILTER_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Apply all-zero digital filter for speech synthesis to signals.
 *
 * The transfer function @f$H(z)@f$ of an all-zero filter is
 * @f[
 *   H(z) = \displaystyle\sum_{m=0}^M b(m)z^{-m},
 * @f]
 * where @f$M@f$ is the order of filter.
 *
 * Given the @f$M@f$-th order filter coefficients,
 * @f[
 *   \begin{array}{cccc}
 *     b(0), & b(1), & \ldots, & b(M),
 *   \end{array}
 * @f]
 * an output signal is obtained by applying @f$H(z)@f$ to an input signal in
 * time domain.
 */
class AllZeroDigitalFilter {
 public:
  /**
   * Buffer for AllZeroDigitalFilter class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    std::vector<double> d_;

    friend class AllZeroDigitalFilter;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_filter_order Order of filter coefficients, @f$M@f$.
   * @param[in] transposition If true, use transposed form filter.
   */
  AllZeroDigitalFilter(int num_filter_order, bool transposition);

  virtual ~AllZeroDigitalFilter() {
  }

  /**
   * @return Order of coefficients.
   */
  int GetNumFilterOrder() const {
    return num_filter_order_;
  }

  /**
   * @return True if transposed form is used.
   */
  bool IsTransposition() const {
    return transposition_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] filter_coefficients @f$M@f$-th order FIR filter coefficients.
   * @param[in] filter_input Input signal.
   * @param[out] filter_output Output signal.
   * @param[in,out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& filter_coefficients, double filter_input,
           double* filter_output, AllZeroDigitalFilter::Buffer* buffer) const;

  /**
   * @param[in] filter_coefficients @f$M@f$-th order FIR filter coefficients.
   * @param[in,out] input_and_output Input/output signal.
   * @param[in,out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& filter_coefficients,
           double* input_and_output,
           AllZeroDigitalFilter::Buffer* buffer) const;

 private:
  const int num_filter_order_;
  const bool transposition_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(AllZeroDigitalFilter);
};

}  // namespace sptk

#endif  // SPTK_FILTER_ALL_ZERO_DIGITAL_FILTER_H_
