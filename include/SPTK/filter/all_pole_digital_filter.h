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

#ifndef SPTK_FILTER_ALL_POLE_DIGITAL_FILTER_H_
#define SPTK_FILTER_ALL_POLE_DIGITAL_FILTER_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Apply all-pole digital filter for speech synthesis to signals.
 *
 * The transfer function @f$H(z)@f$ of an all-pole filter is
 * @f[
 *   H(z) = \frac{K}{1 + \displaystyle\sum_{m=1}^M a(m)z^{-m}},
 * @f]
 * where @f$M@f$ is the order of filter and @f$K@f$ is the gain.
 *
 * Given the @f$M@f$-th order filter coefficients,
 * @f[
 *   \begin{array}{cccc}
 *     K, & a(1), & \ldots, & a(M),
 *   \end{array}
 * @f]
 * an output signal is obtained by applying @f$H(z)@f$ to an input signal in
 * time domain.
 */
class AllPoleDigitalFilter {
 public:
  /**
   * Buffer for AllPoleDigitalFilter class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    std::vector<double> d_;

    friend class AllPoleDigitalFilter;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_filter_order Order of filter coefficients, @f$M@f$.
   * @param[in] transposition If true, use transposed form filter.
   */
  AllPoleDigitalFilter(int num_filter_order, bool transposition);

  virtual ~AllPoleDigitalFilter() {
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
   * @param[in] filter_coefficients @f$M@f$-th order LPC coefficients.
   * @param[in] filter_input Input signal.
   * @param[out] filter_output Output signal.
   * @param[in,out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& filter_coefficients, double filter_input,
           double* filter_output, AllPoleDigitalFilter::Buffer* buffer) const;

  /**
   * @param[in] filter_coefficients @f$M@f$-th order LPC coefficients.
   * @param[in,out] input_and_output Input/output signal.
   * @param[in,out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& filter_coefficients,
           double* input_and_output,
           AllPoleDigitalFilter::Buffer* buffer) const;

 private:
  const int num_filter_order_;
  const bool transposition_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(AllPoleDigitalFilter);
};

}  // namespace sptk

#endif  // SPTK_FILTER_ALL_POLE_DIGITAL_FILTER_H_
