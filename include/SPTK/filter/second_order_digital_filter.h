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

#ifndef SPTK_FILTER_SECOND_ORDER_DIGITAL_FILTER_H_
#define SPTK_FILTER_SECOND_ORDER_DIGITAL_FILTER_H_

#include "SPTK/filter/infinite_impulse_response_digital_filter.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Apply second-order digital filter.
 *
 * The second-order digital fiter is represented as
 * @f{eqnarray}{
 *   H(z) &=& \frac{(1-r_2 e^{j\theta_2}z^{-1})(1-r_2 e^{-j\theta_2}z^{-1})}
 *                 {(1-r_1 e^{j\theta_1}z^{-1})(1-r_1 e^{-j\theta_1}z^{-1})} \\
 *        &=& \frac{1 - 2r_2\cos\theta_2 z^{-1} + r_2^2 z^{-2}}
 *                 {1 - 2r_1\cos\theta_1 z^{-1} + r_1^2 z^{-2}}.
 * @f}
 * The relation between radius @f$r_{(\cdot)}@f$ and the 3-dB bandwidth
 * @f$B_{(\cdot)}@f$ is given by
 * @f{eqnarray}{
 *   r_1 &=& e^{-\pi B_1 / F_s}, \\
 *   r_2 &=& e^{-\pi B_2 / F_s},
 * @f}
 * where @f$F_s@f$ is the sampling rate. The angle @f$\theta_{(\cdot)}@f$ is
 * @f{eqnarray}{
 *   \theta_1 &=& 2\pi F_1 / F_s, \\
 *   \theta_2 &=& 2\pi F_2 / F_s.
 * @f}
 *
 * [1] J. O. Smith, &quot;Relating Pole Radius to Bandwidth,&quot; in
 *     Introduction to digital filters with audio applications,
 *     https://ccrma.stanford.edu/~jos/fp/Relating_Pole_Radius_Bandwidth.html
 */
class SecondOrderDigitalFilter {
 public:
  /**
   * Filter type.
   */
  enum FilterType {
    kPole = 0,
    kZero,
  };

  /**
   * Buffer for SecondOrderDigitalFilter class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    InfiniteImpulseResponseDigitalFilter::Buffer buffer;

    friend class SecondOrderDigitalFilter;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] filter_type Pole or zero.
   * @param[in] frequency Center frequency in Hz.
   * @param[in] bandwidth Bandwidth in Hz.
   * @param[in] sampling_rate Sampling rate in Hz.
   */
  SecondOrderDigitalFilter(FilterType filter_type, double frequency,
                           double bandwidth, double sampling_rate);

  /**
   * @param[in] pole_frequency Pole frequency in Hz.
   * @param[in] pole_bandwidth Pole bandwidth in Hz.
   * @param[in] zero_frequency Zero frequency in Hz.
   * @param[in] zero_bandwidth Zero bandwidth in Hz.
   * @param[in] sampling_rate Sampling rate in Hz.
   */
  SecondOrderDigitalFilter(double pole_frequency, double pole_bandwidth,
                           double zero_frequency, double zero_bandwidth,
                           double sampling_rate);

  virtual ~SecondOrderDigitalFilter() {
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
           SecondOrderDigitalFilter::Buffer* buffer) const;

  /**
   * @param[in,out] input_and_output Input/output signal.
   * @param[in,out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(double* input_and_output,
           SecondOrderDigitalFilter::Buffer* buffer) const;

 private:
  const InfiniteImpulseResponseDigitalFilter filter_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(SecondOrderDigitalFilter);
};

}  // namespace sptk

#endif  // SPTK_FILTER_SECOND_ORDER_DIGITAL_FILTER_H_
