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
class DeltaCalculation : public InputSourceInterface {
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
   * @return Output size.
   */
  virtual int GetSize() const {
    return (num_order_ + 1) * num_delta_;
  }

  /**
   * @return True if this object is valid.
   */
  virtual bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[out] delta Delta components.
   * @return True on success, false on failure.
   */
  virtual bool Get(std::vector<double>* delta);

 private:
  struct Buffer {
    std::vector<std::vector<double> > statics;
    int pointer;
    int count_down;
    bool first;
  };

  bool Forward();

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
