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

#ifndef SPTK_CONVERSION_CEPSTRUM_TO_MINIMUM_PHASE_IMPULSE_RESPONSE_H_
#define SPTK_CONVERSION_CEPSTRUM_TO_MINIMUM_PHASE_IMPULSE_RESPONSE_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Convert minimum phase impulse response to cepstrum.
 *
 * The input is the @f$M_1@f$-th order cepstral coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     c(0), & c(1), & \ldots, & c(M_1),
 *   \end{array}
 * @f]
 * and the output is the @f$M_2@f$-th order impulse response:
 * @f[
 *   \begin{array}{cccc}
 *     h(0), & h(1), & \ldots, & h(M_2).
 *   \end{array}
 * @f]
 * The truncated impulse response is obtained from the following recursion
 * formula:
 * @f[
 *   h(n) = \left\{ \begin{array}{ll}
 *     \exp c(0), & n = 0 \\
 *     \displaystyle\sum_{k=1}^{n} \frac{k}{n} c(k) h(n-k). & n > 0
 *   \end{array} \right.
 * @f]
 *
 * [1] A. V. Oppenheim and R. W. Schafer, &quot;Discrete-time signal
 *     processing, 3rd edition,&quot; Prentice-Hall Signal Processing Series,
 *     pp. 985-986, 2009.
 */
class CepstrumToMinimumPhaseImpulseResponse {
 public:
  /**
   * @param[in] num_input_order Order of cepstral coefficients, @f$M_1@f$.
   * @param[in] num_output_order Order of impulse response, @f$M_2@f$.
   */
  CepstrumToMinimumPhaseImpulseResponse(int num_input_order,
                                        int num_output_order);

  virtual ~CepstrumToMinimumPhaseImpulseResponse() {
  }

  /**
   * @return Order of cepstral coefficients.
   */
  int GetNumInputOrder() const {
    return num_input_order_;
  }

  /**
   * @return Order of impulse response.
   */
  int GetNumOutputOrder() const {
    return num_output_order_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] cepstrum @f$M_1@f$-th order cepstral coefficients.
   * @param[out] minimum_phase_impulse_response @f$M_2@f$-th order impulse
   *             response.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& cepstrum,
           std::vector<double>* minimum_phase_impulse_response) const;

 private:
  const int num_input_order_;
  const int num_output_order_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(CepstrumToMinimumPhaseImpulseResponse);
};

}  // namespace sptk

#endif  // SPTK_CONVERSION_CEPSTRUM_TO_MINIMUM_PHASE_IMPULSE_RESPONSE_H_
