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

#ifndef SPTK_FILTER_MGLSA_DIGITAL_FILTER_H_
#define SPTK_FILTER_MGLSA_DIGITAL_FILTER_H_

#include <vector>  // std::vector

#include "SPTK/filter/mlsa_digital_filter.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Apply MGLSA digital filter for speech synthesis to signals.
 *
 * The transfer function @f$H(z)@f$ of an MLSA filter is
 * @f{eqnarray}{
 *   H(z) &=& s_\gamma^{-1}
 *            \left( \sum_{m=0}^M b'_\gamma(m) \varPhi_m(z) \right) \\
 *        &\equiv& K \cdot D(z)
 * @f}
 * where
 * @f[
 *   \varPhi_m(z) = \left\{ \begin{array}{ll}
 *     1, & m = 0 \\
 *     \dfrac{(1-\alpha^2)z^{-1}}{1-\alpha z^{-1}} \tilde{z}^{-(m-1)}, & m \ge 1
 *   \end{array} \right.
 * @f]
 * and
 * @f{eqnarray}{
 *      K &=& s_\gamma^{-1} \left( b'_\gamma(0) \right), \\
 *   D(z) &=& s_\gamma^{-1} \left(\sum_{m=1}^M b'_\gamma(m) \varPhi_m(z)\right).
 * @f}
 * The filter @f$D(z)@f$ is implemented by cascading filter @f$1/B(z)@f$:
 * @f{eqnarray}{
 *   D(z) &=& \left( \frac{1}{B(z)} \right)^C \\
 *        &=& \left( \frac{1}
 *            {1 + \gamma \sum_{m=1}^M b'_\gamma(m) \varPhi_m(z)} \right)^C .
 * @f}
 *
 * @image html mglsadf_2.png
 *
 * The filter coeffcients @f$b'_\gamma(m)@f$ are obtained by applying gain
 * normalization to the MLSA filter coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     b(0), & b(1), & \ldots, & b(M).
 *   \end{array}
 * @f]
 * An output signal is obtained by applying @f$H(z)@f$ to an input signal in
 * time domain.
 */
class MglsaDigitalFilter {
 public:
  /**
   * Buffer for MglsaDigitalFilter class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    std::vector<double> signals_;
    MlsaDigitalFilter::Buffer mlsa_digital_filter_buffer_;

    friend class MglsaDigitalFilter;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_filter_order Order of filter coefficients, @f$M@f$.
   * @param[in] num_pade_order Order of Pade approximation, @f$L@f$.
   * @param[in] num_stage Number of stages, @f$C@f$. If zero, MLSA filter is
   *            used.
   * @param[in] alpha All-pass constant, @f$\alpha@f$.
   * @param[in] transposition If true, use transposed form filter.
   */
  MglsaDigitalFilter(int num_filter_order, int num_pade_order, int num_stage,
                     double alpha, bool transposition);

  virtual ~MglsaDigitalFilter() {
  }

  /**
   * @return Order of coefficients.
   */
  int GetNumFilterOrder() const {
    return num_filter_order_;
  }

  /**
   * @return Order of Pade approximation.
   */
  int GetNumPadeOrder() const {
    return mlsa_digital_filter_.GetNumPadeOrder();
  }

  /**
   * @return Number of stages.
   */
  int GetNumStage() const {
    return num_stage_;
  }

  /**
   * @return All-pass constant.
   */
  double GetAlpha() const {
    return alpha_;
  }

  /**
   * @return True if transposed form is used.
   */
  bool GetTranspositionFlag() const {
    return transposition_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] filter_coefficients @f$M@f$-th order MGLSA filter coefficients.
   * @param[in] filter_input Input signal.
   * @param[out] filter_output Output signal.
   * @param[in,out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& filter_coefficients, double filter_input,
           double* filter_output, MglsaDigitalFilter::Buffer* buffer) const;

  /**
   * @param[in] filter_coefficients @f$M@f$-th order MGLSA filter coefficients.
   * @param[in,out] input_and_output Input/output signal.
   * @param[in,out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& filter_coefficients,
           double* input_and_output, MglsaDigitalFilter::Buffer* buffer) const;

 private:
  const int num_filter_order_;
  const int num_stage_;
  const double alpha_;
  const bool transposition_;

  const MlsaDigitalFilter mlsa_digital_filter_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(MglsaDigitalFilter);
};

}  // namespace sptk

#endif  // SPTK_FILTER_MGLSA_DIGITAL_FILTER_H_
