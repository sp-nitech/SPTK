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

#ifndef SPTK_MATH_SECOND_ORDER_ALL_PASS_FREQUENCY_TRANSFORM_H_
#define SPTK_MATH_SECOND_ORDER_ALL_PASS_FREQUENCY_TRANSFORM_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Transform a minimum phase sequence into a frequency-warped sequence.
 */
class SecondOrderAllPassFrequencyTransform {
 public:
  /**
   * @param[in] num_input_order Order of input, @f$M_1@f$.
   * @param[in] num_output_order Order of output, @f$M_2@f$.
   * @param[in] fft_length FFT length used to make conversion matrix.
   * @param[in] alpha Frequency warping factor, @f$\alpha@f$.
   * @param[in] theta Frequency emphasis factor, @f$\theta@f$.
   */
  SecondOrderAllPassFrequencyTransform(int num_input_order,
                                       int num_output_order, int fft_length,
                                       double alpha, double theta);

  virtual ~SecondOrderAllPassFrequencyTransform() {
  }

  /**
   * @return Order of input.
   */
  int GetNumInputOrder() const {
    return num_input_order_;
  }

  /**
   * @return Order of output.
   */
  int GetNumOutputOrder() const {
    return num_output_order_;
  }

  /**
   * @return Frequency warping factor.
   */
  double GetAlpha() const {
    return alpha_;
  }

  /**
   * @return Frequency emphasis factor.
   */
  double GetTheta() const {
    return theta_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] minimum_phase_sequence @f$M_1@f$-th order input sequence.
   * @param[out] warped_sequence @f$M_2@f$-th order output sequence.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& minimum_phase_sequence,
           std::vector<double>* warped_sequence) const;

 private:
  const int num_input_order_;
  const int num_output_order_;
  const double alpha_;
  const double theta_;

  bool is_valid_;

  std::vector<std::vector<double> > conversion_matrix_;

  DISALLOW_COPY_AND_ASSIGN(SecondOrderAllPassFrequencyTransform);
};

}  // namespace sptk

#endif  // SPTK_MATH_SECOND_ORDER_ALL_PASS_FREQUENCY_TRANSFORM_H_
