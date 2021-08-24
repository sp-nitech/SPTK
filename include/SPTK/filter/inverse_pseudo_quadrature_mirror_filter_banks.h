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

#ifndef SPTK_FILTER_INVERSE_PSEUDO_QUADRATURE_MIRROR_FILTER_BANKS_H_
#define SPTK_FILTER_INVERSE_PSEUDO_QUADRATURE_MIRROR_FILTER_BANKS_H_

#include <vector>  // std::vector

#include "SPTK/filter/all_zero_digital_filter.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Reconstruct signal from subband signals.
 *
 * The input is the @f$K@f$ subband signals:
 * @f[
 *   \begin{array}{cccc}
 *     x_0(t), & x_1(t), & \ldots, & x_{K-1}(t),
 *   \end{array}
 * @f]
 * and the output is the signal @f$x(t)@f$.
 * The impulse responses of the analysis filters are cosine-modulated versions
 * of the prototype filter @f$h(n)@f$:
 * @f[
 *   f_k(n) = 2h(n) \cos \left(
 *     (2k+1) \frac{\pi}{2K} \left( n-\frac{M}{2} \right) - (-1)^k \frac{\pi}{4}
 *   \right).
 * @f]
 * where @f$M@f$ is the filter order.
 * In the implemented algorithm, the prototype filter @f$h(n)@f$ is represented
 * as
 * @f[
 *   h(n) = g(n) w(n),
 * @f]
 * where @f$w(n)@f$ is the Kaiser window and
 * @f[
 *   g(n) = \frac{\sin \left( n-\frac{M}{2} \right) \omega}
 *               {\pi \left( n-\frac{M}{2} \right)}
 * @f]
 * is the shifted impulse response of an ideal lowpass filter. The optimal
 * angular frequency @f$\omega@f$ is calculated based on a simple algorithm.
 */
class InversePseudoQuadratureMirrorFilterBanks {
 public:
  /**
   * Buffer for InversePseudoQuadratureMirrorFilterBanks class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
      for (std::vector<AllZeroDigitalFilter::Buffer*>::iterator itr(
               buffer_for_all_zero_filter_.begin());
           itr != buffer_for_all_zero_filter_.end(); ++itr) {
        delete (*itr);
      }
    }

   private:
    std::vector<AllZeroDigitalFilter::Buffer*> buffer_for_all_zero_filter_;

    friend class InversePseudoQuadratureMirrorFilterBanks;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_subband Number of subbands, @f$K@f$.
   * @param[in] num_filter_order Order of filter, @f$M@f$.
   * @param[in] attenuation Stopband attenuation in dB.
   * @param[in] num_iteration Number of iterations.
   * @param[in] convergence_threshold Convergence threshold.
   * @param[in] initial_step_size Initial step size.
   */
  InversePseudoQuadratureMirrorFilterBanks(int num_subband,
                                           int num_filter_order,
                                           double attenuation,
                                           int num_iteration,
                                           double convergence_threshold,
                                           double initial_step_size);

  virtual ~InversePseudoQuadratureMirrorFilterBanks() {
  }

  /**
   * @return Number of subbands.
   */
  int GetNumSubband() const {
    return num_subband_;
  }

  /**
   * @return Order of filter.
   */
  int GetNumFilterOrder() const {
    return all_zero_filter_.GetNumFilterOrder();
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @return True if built filter is in convergence point.
   */
  bool IsConverged() const {
    return is_converged_;
  }

  /**
   * @param[in] input Input subband signals.
   * @param[out] output Output signal.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& input, double* output,
           InversePseudoQuadratureMirrorFilterBanks::Buffer* buffer) const;

 private:
  const int num_subband_;
  const AllZeroDigitalFilter all_zero_filter_;

  bool is_valid_;
  bool is_converged_;
  std::vector<std::vector<double> > filter_banks_;

  DISALLOW_COPY_AND_ASSIGN(InversePseudoQuadratureMirrorFilterBanks);
};

}  // namespace sptk

#endif  // SPTK_FILTER_INVERSE_PSEUDO_QUADRATURE_MIRROR_FILTER_BANKS_H_
