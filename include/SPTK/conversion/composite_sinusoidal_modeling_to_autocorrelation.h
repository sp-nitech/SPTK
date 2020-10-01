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

#ifndef SPTK_CONVERSION_COMPOSITE_SINUSOIDAL_MODELING_TO_AUTOCORRELATION_H_
#define SPTK_CONVERSION_COMPOSITE_SINUSOIDAL_MODELING_TO_AUTOCORRELATION_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Transform CSM parameters to autocorrelation.
 *
 * The input is the \f$2N\f$ CSM parameters:
 * \f[
 *   \begin{array}{cccc}
 *     \omega(1), & \omega(2), & \ldots, & \omega(N), \\
 *     m(1), & m(2), & \ldots, & m(N),
 *   \end{array}
 * \f]
 * where \f$\omega(n)\f$ is the CSM frequency and \f$m(n)\f$ is the CSM
 * intensity. The output is the \f$(2N-1)\f$-th order biased sample
 * autocorrelation coefficients:
 * \f[
 *   \begin{array}{cccc}
 *     v(0), & v(1), & \ldots, & v(2N-1).
 *   \end{array}
 * \f]
 * The sample autocorrelation is given by
 * \f[
 *     v(l) = \sum_{i=1}^N m(i) \cos(l \, \omega(i)).
 * \f]
 *
 * [1] S. Sagayama and F. Itakura, &quot;Duality theory of composite sinusoidal
 *     modeling and linear prediction,&quot; Proc. of ICASSP 1986,
 *     pp. 1261-1264, 1986.
 */
class CompositeSinusoidalModelingToAutocorrelation {
 public:
  /**
   * @param[in] num_sine_wave Number of sine waves, \f$N\f$.
   */
  explicit CompositeSinusoidalModelingToAutocorrelation(int num_sine_wave);

  virtual ~CompositeSinusoidalModelingToAutocorrelation() {
  }

  /**
   * @return Number of sine waves.
   */
  int GetNumSineWaves() const {
    return num_sine_wave_;
  }

  /**
   * @return True if this obejct is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] composite_sinusoidal_modeling CSM parameters.
   * @param[out] autocorrelation Autocorrelation coefficients.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& composite_sinusoidal_modeling,
           std::vector<double>* autocorrelation) const;

  /**
   * @param[in,out] input_and_output Input and output.
   * @return True on success, false on failure.
   */
  bool Run(std::vector<double>* input_and_output) const;

 private:
  const int num_sine_wave_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(CompositeSinusoidalModelingToAutocorrelation);
};

}  // namespace sptk

#endif  // SPTK_CONVERSION_COMPOSITE_SINUSOIDAL_MODELING_TO_AUTOCORRELATION_H_
