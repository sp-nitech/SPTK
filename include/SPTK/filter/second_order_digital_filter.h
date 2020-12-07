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
