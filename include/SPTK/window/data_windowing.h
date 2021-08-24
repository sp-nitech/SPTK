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

#ifndef SPTK_WINDOW_DATA_WINDOWING_H_
#define SPTK_WINDOW_DATA_WINDOWING_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"
#include "SPTK/window/window_interface.h"

namespace sptk {

/**
 * Apply a window function.
 *
 * The input is the @f$L_1@f$-length data:
 * @f[
 *   \begin{array}{cccc}
 *     x(0), & x(1), & \ldots, & x(L_1-1),
 *   \end{array}
 * @f]
 * and the output is the @f$L_2@f$-th length data:
 * @f[
 *   \begin{array}{cccc}
 *     x'(0), & x'(1), & \ldots, & x'(L_2-1).
 *   \end{array}
 * @f]
 * where
 * @f[
 *   x'(l) = \left\{ \begin{array}{cc}
 *     x(l)w(l), & l < L_1 \\
 *     0.        & L_1 \le l
 *   \end{array} \right.
 * @f]
 * and @f$w(\cdot)@f$ is a window.
 *
 * The window @f$w(\cdot)@f$ is normalized depending on the given normalization
 * type. There are two types of normalization:
 * @f[
 *   \sum_{l=0}^{L_1} w^2(l) = 1
 * @f]
 * and
 * @f[
 *   \sum_{l=0}^{L_1} w(l) = 1.
 * @f]
 */
class DataWindowing {
 public:
  /**
   * Normalization type.
   */
  enum NormalizationType {
    kNone = 0,
    kPower,
    kMagnitude,
    kNumNormalizationTypes
  };

  /**
   * @param[in] window @f$L_1@f$-length window to be used.
   * @param[in] output_length Output length, @f$L_2@f$.
   * @param[in] normalization_type Type of normalization.
   */
  DataWindowing(WindowInterface* window, int output_length,
                NormalizationType normalization_type);

  virtual ~DataWindowing() {
  }

  /**
   * @return Input length.
   */
  int GetInputLength() const {
    return input_length_;
  }

  /**
   * @return Output length.
   */
  int GetOutputLength() const {
    return output_length_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] data @f$L_1@f$-length input data.
   * @param[out] windowed_data @f$L_2@f$-length output data.
   */
  bool Run(const std::vector<double>& data,
           std::vector<double>* windowed_data) const;

 private:
  const int input_length_;
  const int output_length_;

  bool is_valid_;

  std::vector<double> window_;

  DISALLOW_COPY_AND_ASSIGN(DataWindowing);
};

}  // namespace sptk

#endif  // SPTK_WINDOW_DATA_WINDOWING_H_
