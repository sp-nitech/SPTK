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

#ifndef SPTK_GENERATION_DELTA_CALCULATION_H_
#define SPTK_GENERATION_DELTA_CALCULATION_H_

#include <vector>  // std::vector

#include "SPTK/input/input_source_interface.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Calculate derivatives.
 *
 * The input is the @f$M@f$-th order static feature components:
 * @f[
 *   \begin{array}{cccc}
 *     x_t(0), & x_t(1), & \ldots, & x_t(M),
 *   \end{array}
 * @f]
 * and the output is composed of the set of dynamic feature components:
 * @f[
 *   \begin{array}{cccccc}
 *     \Delta^{(1)} x_t(0), & \ldots, & \Delta^{(1)} x_t(M), &
 *     \Delta^{(2)} x_t(0), & \ldots, & \Delta^{(D)} x_t(M).
 *   \end{array}
 * @f]
 *
 * The derivatives are derived as
 * @f[
 *   \Delta^{(d)} x(m) = \sum_{\tau=-L^{(d)}}^{L^{(d)}}
 *      w^{(d)}_{\tau} x_{t+\tau}(m)
 * @f]
 * where @f$w^{(d)}@f$ is the @f$d@f$-th window coefficients and @f$L^{(d)}@f$
 * is half the width of the window.
 */
class DeltaCalculation {
 public:
  /**
   * @param[in] num_order Order of coefficients, @f$M@f$.
   * @param[in] window_coefficients Window coefficients.
   *            e.g.) { {1.0}, {-0.5, 0.0, 0.5} }
   * @param[in] input_source Static components sequence.
   * @param[in] use_magic_number Whether to use a magic number.
   * @param[in] magic_number A magic number.
   */
  DeltaCalculation(int num_order,
                   const std::vector<std::vector<double> >& window_coefficients,
                   InputSourceInterface* input_source, bool use_magic_number,
                   double magic_number = 0.0);

  virtual ~DeltaCalculation() {
  }

  /**
   * @return Order of coefficients.
   */
  int GetNumOrder() const {
    return num_order_;
  }

  /**
   * @return Magic number.
   */
  double GetMagicNumber() const {
    return magic_number_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[out] delta Delta components.
   * @return True on success, false on failure.
   */
  bool Get(std::vector<double>* delta);

 private:
  struct Buffer {
    std::vector<std::vector<double> > statics;
    int pointer;
    int count_down;
    bool first;
  };

  bool Lookahead();

  int GetPointerIndex(int move);

  const int num_order_;
  const int num_delta_;
  const std::vector<std::vector<double> > window_coefficients_;
  InputSourceInterface* input_source_;
  const bool use_magic_number_;
  const double magic_number_;

  bool is_valid_;

  int max_window_width_;
  std::vector<int> lefts_;
  std::vector<int> rights_;

  Buffer buffer_;

  DISALLOW_COPY_AND_ASSIGN(DeltaCalculation);
};

}  // namespace sptk

#endif  // SPTK_GENERATION_DELTA_CALCULATION_H_
