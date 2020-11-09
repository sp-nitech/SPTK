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
   * @return True if this obejct is valid.
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
