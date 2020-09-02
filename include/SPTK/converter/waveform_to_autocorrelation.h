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

#ifndef SPTK_CONVERTER_WAVEFORM_TO_AUTOCORRELATION_H_
#define SPTK_CONVERTER_WAVEFORM_TO_AUTOCORRELATION_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Calculate autocorrelation.
 *
 * The input is the framed waveform signal:
 * \f[
 *   \begin{array}{cccc}
 *     x(0), & x(1), & \ldots, & x(L-1),
 *   \end{array}
 * \f]
 * where \f$L\f$ is the frame length. The output is the \f$M\f$-th order
 * autocorrelation coefficients:
 * \f[
 *   \begin{array}{cccc}
 *     r(0), & r(1), & \ldots, & r(M).
 *   \end{array}
 * \f]
 * The autocorrelation is given by
 * \f[
 *   r(m) = \sum_{l=0}^{L-1-m} x(l)x(l+m),
 * \f]
 * where \f$m\f$ is the lag.
 */
class WaveformToAutocorrelation {
 public:
  /**
   * @param[in] frame_length Frame length, \f$L\f$.
   * @param[in] num_order Order of autocorrelation, \f$M\f$.
   */
  WaveformToAutocorrelation(int frame_length, int num_order);

  virtual ~WaveformToAutocorrelation() {
  }

  /**
   * @return Frame length.
   */
  int GetFrameLength() const {
    return frame_length_;
  }

  /**
   * @return Order of autocorrelation.
   */
  int GetNumOrder() const {
    return num_order_;
  }

  /**
   * @return True if this obejct is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] waveform \f$L\f$-length framed waveform.
   * @param[out] autocorrelation \f$M\f$-th order autocorrelation coefficients.
   */
  bool Run(const std::vector<double>& waveform,
           std::vector<double>* autocorrelation) const;

 private:
  const int frame_length_;
  const int num_order_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(WaveformToAutocorrelation);
};

}  // namespace sptk

#endif  // SPTK_CONVERTER_WAVEFORM_TO_AUTOCORRELATION_H_
