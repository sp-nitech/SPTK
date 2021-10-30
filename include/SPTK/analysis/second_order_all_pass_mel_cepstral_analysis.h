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

#ifndef SPTK_ANALYSIS_SECOND_ORDER_ALL_PASS_MEL_CEPSTRAL_ANALYSIS_H_
#define SPTK_ANALYSIS_SECOND_ORDER_ALL_PASS_MEL_CEPSTRAL_ANALYSIS_H_

#include <vector>  // std::vector

#include "SPTK/math/real_valued_fast_fourier_transform.h"
#include "SPTK/math/real_valued_inverse_fast_fourier_transform.h"
#include "SPTK/math/second_order_all_pass_frequency_transform.h"
#include "SPTK/math/second_order_all_pass_inverse_frequency_transform.h"
#include "SPTK/math/toeplitz_plus_hankel_system_solver.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Calculate mel-cepstrum from periodogram.
 *
 * The input is the half of periodogram:
 * @f[
 *   \begin{array}{cccc}
 *     |X(0)|^2, & |X(1)|^2, & \ldots, & |X(N/2)|^2,
 *   \end{array}
 * @f]
 * where @f$N@f$ is the FFT length. The output is the @f$M@f$-th order
 * mel-cepstral coefficients:
 * @f[
 *   \begin{array}{cccc}
 *     \tilde{c}(0), & \tilde{c}(1), & \ldots, & \tilde{c}(M).
 *   \end{array}
 * @f]
 *
 * In the mel-cepstral analysis based on the second order all-pass function,
 * the spectrum of speech signal is modeled by @f$M@f$-th order mel-cepstral
 * coefficients as follows:
 * @f[
 *   H(z) = \exp \sum_{m=0}^M \tilde{c}(m)
 *     \displaystyle\frac{A^m(z) + A^m(z^{-1})}{2},
 * @f]
 * where
 * @f[
 *   A(z) = \left( \frac{z^{-2} - 2\alpha\cos\theta z^{-1} + \alpha^2}
 *     {1 - 2\alpha \cos\theta z^{-1} + \alpha^2 z^{-2}} \right)^{1/2}.
 * @f]
 * The phase characteristic of the all-pass function is controlled by the two
 * parameters: @f$\alpha@f$ and @f$\theta@f$.
 *
 * Note that the implemenation is based on an unpublished paper.
 */
class SecondOrderAllPassMelCepstralAnalysis {
 public:
  /**
   * Buffer for SecondOrderAllPassMelCepstralAnalysis class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    std::vector<double> log_periodogram_;
    std::vector<double> cepstrum_;
    std::vector<double> imag_part_output_;
    std::vector<std::vector<double> > b_;
    std::vector<double> d_;
    std::vector<double> r_;
    std::vector<double> rt_;
    std::vector<double> rr_;
    std::vector<double> ra_;
    std::vector<double> gradient_;

    RealValuedFastFourierTransform::Buffer buffer_for_fourier_transform_;
    RealValuedInverseFastFourierTransform::Buffer
        buffer_for_inverse_fourier_transform_;
    ToeplitzPlusHankelSystemSolver::Buffer buffer_for_system_solver_;

    friend class SecondOrderAllPassMelCepstralAnalysis;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] fft_length Number of FFT bins, @f$N@f$.
   * @param[in] num_order Order of cepstral coefficients, @f$M@f$.
   * @param[in] accuracy_factor Accuracy of frequency transform. This must be
   *            a power of two.
   * @param[in] alpha Frequency warping factor, @f$\alpha@f$.
   * @param[in] theta Frequency emphasis factor, @f$\theta@f$.
   * @param[in] num_iteration Number of iterations of Newton method, @f$J@f$.
   * @param[in] convergence_threshold Convergence threshold, @f$\epsilon@f$.
   */
  SecondOrderAllPassMelCepstralAnalysis(int fft_length, int num_order,
                                        int accuracy_factor, double alpha,
                                        double theta, int num_iteration,
                                        double convergence_threshold);

  virtual ~SecondOrderAllPassMelCepstralAnalysis() {
  }

  /**
   * @return FFT length.
   */
  int GetFftLength() const {
    return fft_length_;
  }

  /**
   * @return Order of coefficients.
   */
  int GetNumOrder() const {
    return num_order_;
  }

  /**
   * @return All-pass constant.
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
   * @return A factor for making a conversion matrix of frequency transform.
   */
  int AccuracyFactor() const {
    return accuracy_factor_;
  }

  /**
   * @return Number of iterations.
   */
  int GetNumIteration() const {
    return num_iteration_;
  }

  /**
   * @return Convergence threshold.
   */
  double GetConvergenceThreshold() const {
    return convergence_threshold_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] periodogram @f$(N/2+1)@f$-length periodogram.
   * @param[out] mel_cepstrum @f$M@f$-th order mel-cepstral coefficients.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& periodogram,
           std::vector<double>* mel_cepstrum,
           SecondOrderAllPassMelCepstralAnalysis::Buffer* buffer) const;

 private:
  const int fft_length_;
  const int num_order_;
  const int accuracy_factor_;
  const double alpha_;
  const double theta_;
  const int num_iteration_;
  const double convergence_threshold_;

  const RealValuedFastFourierTransform fourier_transform_;
  const RealValuedInverseFastFourierTransform inverse_fourier_transform_;
  const SecondOrderAllPassFrequencyTransform frequency_transform_;
  const SecondOrderAllPassInverseFrequencyTransform
      inverse_frequency_transform_;
  const ToeplitzPlusHankelSystemSolver toeplitz_plus_hankel_system_solver_;

  bool is_valid_;

  std::vector<double> alpha_vector_;

  DISALLOW_COPY_AND_ASSIGN(SecondOrderAllPassMelCepstralAnalysis);
};

}  // namespace sptk

#endif  // SPTK_ANALYSIS_SECOND_ORDER_ALL_PASS_MEL_CEPSTRAL_ANALYSIS_H_
