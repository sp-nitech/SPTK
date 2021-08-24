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

#ifndef SPTK_FILTER_ALL_POLE_LATTICE_DIGITAL_FILTER_H_
#define SPTK_FILTER_ALL_POLE_LATTICE_DIGITAL_FILTER_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Apply all-pole lattice filter for speech synthesis.
 *
 * Given the @f$M@f$-th order PARCOR coefficients,
 * @f[
 *   \begin{array}{cccc}
 *     K, & k(1), & \ldots, & k(M),
 *   \end{array}
 * @f]
 * an output signal is obtained by applying the all-pole lattice filter to an
 * input signal in time domain.
 */
class AllPoleLatticeDigitalFilter {
 public:
  /**
   * Buffer for AllPoleLatticeDigitalFilter class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    std::vector<double> d_;

    friend class AllPoleLatticeDigitalFilter;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_filter_order Order of filter coefficients, @f$M@f$.
   */
  explicit AllPoleLatticeDigitalFilter(int num_filter_order);

  virtual ~AllPoleLatticeDigitalFilter() {
  }

  /**
   * @return Order of coefficients.
   */
  int GetNumFilterOrder() const {
    return num_filter_order_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] filter_coefficients @f$M@f$-th order PARCOR coefficients.
   * @param[in] filter_input Input signal.
   * @param[out] filter_output Output signal.
   * @param[in,out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& filter_coefficients, double filter_input,
           double* filter_output,
           AllPoleLatticeDigitalFilter::Buffer* buffer) const;

  /**
   * @param[in] filter_coefficients @f$M@f$-th order PARCOR coefficients.
   * @param[in,out] input_and_output Input/output signal.
   * @param[in,out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& filter_coefficients,
           double* input_and_output,
           AllPoleLatticeDigitalFilter::Buffer* buffer) const;

 private:
  const int num_filter_order_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(AllPoleLatticeDigitalFilter);
};

}  // namespace sptk

#endif  // SPTK_FILTER_ALL_POLE_LATTICE_DIGITAL_FILTER_H_
