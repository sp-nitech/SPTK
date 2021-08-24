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

#ifndef SPTK_FILTER_MLSA_DIGITAL_FILTER_H_
#define SPTK_FILTER_MLSA_DIGITAL_FILTER_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Apply MLSA digital filter for speech synthesis to signals.
 *
 * The transfer function @f$H(z)@f$ of an MLSA filter is
 * @f{eqnarray}{
 *   H(z) &=& \exp \sum_{m=0}^M b(m) \varPhi_m(z) \\
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
 *      K &=& \exp \, b(0), \\
 *   D(z) &=& \exp \sum_{m=1}^M b(m) \varPhi_m(z).
 * @f}
 * The filter @f$D(z)@f$ can be implemented by approximating exponential
 * function as an @f$L@f$-th order rational function @f$R_L(\cdot)@f$ using
 * the modified Pade approximation:
 * @f[
 *   D(z) = \exp F(z) \simeq R_L(F(z)).
 * @f]
 * The block diagram of the basic filter @f$F(z)@f$ is shown as below.
 *
 * @image html mglsadf_1.png
 *
 * To improve the accuracy of the approximation, the basic filter is decomposed
 * as
 * @f[
 *   F(z) = F_1(z) + F_2(z)
 * @f]
 * where
 * @f{eqnarray}{
 *   F_1(z) &=& b(1) \varPhi_1(z), \\
 *   F_2(z) &=& \exp \sum_{m=2}^M b(m) \varPhi_m(z).
 * @f}
 *
 * Given the @f$M@f$-th order MLSA filter coefficients,
 * @f[
 *   \begin{array}{cccc}
 *     b(0), & b(1), & \ldots, & b(M),
 *   \end{array}
 * @f]
 * an output signal is obtained by applying @f$H(z)@f$ to an input signal in
 * time domain.
 */
class MlsaDigitalFilter {
 public:
  /**
   * Buffer for MlsaDigitalFilter class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    std::vector<double> signals_for_basic_filter1_;
    std::vector<double> signals_for_basic_filter2_;
    std::vector<double> signals_for_exp_filter1_;
    std::vector<double> signals_for_exp_filter2_;

    friend class MlsaDigitalFilter;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_filter_order Order of filter coefficients, @f$M@f$.
   * @param[in] num_pade_order Order of Pade approximation, @f$L@f$.
   * @param[in] alpha All-pass constant, @f$\alpha@f$.
   * @param[in] transposition If true, use transposed form filter.
   */
  MlsaDigitalFilter(int num_filter_order, int num_pade_order, double alpha,
                    bool transposition);

  virtual ~MlsaDigitalFilter() {
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
    return num_pade_order_;
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
   * @param[in] filter_coefficients @f$M@f$-th order MLSA filter coefficients.
   * @param[in] filter_input Input signal.
   * @param[out] filter_output Output signal.
   * @param[in,out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& filter_coefficients, double filter_input,
           double* filter_output, MlsaDigitalFilter::Buffer* buffer) const;

  /**
   * @param[in] filter_coefficients @f$M@f$-th order MLSA filter coefficients.
   * @param[in,out] input_and_output Input/output signal.
   * @param[in,out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& filter_coefficients,
           double* input_and_output, MlsaDigitalFilter::Buffer* buffer) const;

 private:
  const int num_filter_order_;
  const int num_pade_order_;
  const double alpha_;
  const bool transposition_;

  bool is_valid_;

  std::vector<double> pade_coefficients_;

  DISALLOW_COPY_AND_ASSIGN(MlsaDigitalFilter);
};

}  // namespace sptk

#endif  // SPTK_FILTER_MLSA_DIGITAL_FILTER_H_
