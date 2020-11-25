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

#ifndef SPTK_FILTER_PSEUDO_QUADRATURE_MIRROR_FILTER_BANKS_H_
#define SPTK_FILTER_PSEUDO_QUADRATURE_MIRROR_FILTER_BANKS_H_

#include <vector>  // std::vector

#include "SPTK/filter/all_zero_digital_filter.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Decompose signal into subband signals.
 *
 * The input is the signal @f$x(t)@f$ and the output is the @f$K@f$ subband
 * signals:
 * @f[
 *   \begin{array}{cccc}
 *     x_0(t), & x_1(t), & \ldots, & x_{K-1}(t),
 *   \end{array}
 * @f]
 * The impulse responses of the analysis filters are cosine-modulated versions
 * of the prototype filter @f$h(n)@f$:
 * @f[
 *   h_k(n) = 2h(n) \cos \left(
 *     (2k+1) \frac{\pi}{2K} \left( n-\frac{M}{2} \right) + (-1)^k \frac{\pi}{4}
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
 *               {\pi \left( n-\frac{M}{2} \right)}.
 * @f]
 * is the shifted impulse response of an ideal lowpass filter. The optimal
 * angular frequency @f$\omega@f$ is calculated based on a simple algorithm.
 */
class PseudoQuadratureMirrorFilterBanks {
 public:
  /**
   * Buffer for PseudoQuadratureMirrorFilterBanks class.
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

    friend class PseudoQuadratureMirrorFilterBanks;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_subband Number of subbands, @f$K@f$.
   * @param[in] num_filter_order Order of filter, @f$M@f$.
   * @param[in] attenuation Stopband attenuation in dB.
   * @param[in] num_iteration Number of iterations.
   * @param[in] convergnce_threshold Convergence threshold, @f$\epsilon@f$.
   * @param[in] initial_step_size Initial step size, @f$\Delta@f$.
   */
  PseudoQuadratureMirrorFilterBanks(int num_subbands, int num_filter_order,
                                    double attenuation, int num_iteration,
                                    double convergence_threshold,
                                    double initial_step_size);

  virtual ~PseudoQuadratureMirrorFilterBanks() {
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
   * @param[in] input Input signal.
   * @param[out] output Output subband signals.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(double input, std::vector<double>* output,
           PseudoQuadratureMirrorFilterBanks::Buffer* buffer) const;

 private:
  const int num_subband_;
  const AllZeroDigitalFilter all_zero_filter_;

  bool is_valid_;

  std::vector<std::vector<double> > filter_banks_;

  DISALLOW_COPY_AND_ASSIGN(PseudoQuadratureMirrorFilterBanks);
};

}  // namespace sptk

#endif  // SPTK_FILTER_PSEUDO_QUADRATURE_MIRROR_FILTER_BANKS_H_
