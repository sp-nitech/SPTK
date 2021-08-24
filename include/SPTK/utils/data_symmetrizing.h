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

#ifndef SPTK_UTILS_DATA_SYMMETRIZING_H_
#define SPTK_UTILS_DATA_SYMMETRIZING_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Symmetrize/desymmetrize data sequence.
 *
 * The input and output takes one of the following forms.
 *
 * Type 0:
 * @f[
 *   \begin{array}{cccc}
 *     x(0), & x(1), & \ldots, & x(L/2).
 *   \end{array}
 * @f]
 * Type 1:
 * @f[
 *   \begin{array}{ccccccc}
 *     x(0), & x(1), & \ldots, & x(L/2), & x(L/2-1), & \ldots, & x(1).
 *   \end{array}
 * @f]
 * Type 2:
 * @f[
 *   \begin{array}{cccccccc}
 *     \frac{1}{2} x(L/2), & x(L/2-1), & \ldots, & x(0),
 *     & x(1), & \ldots, & x(L/2-1), & \frac{1}{2} x(L/2).
 *   \end{array}
 * @f]
 * Type 3:
 * @f[
 *   \begin{array}{cccccccc}
 *     x(L/2), & x(L/2-1), & \ldots, & x(0),
 *     & x(1), & \ldots, & x(L/2-1), & x(L/2),
 *   \end{array}
 * @f]
 * where @f$L@f$ must be even.
 */
class DataSymmetrizing {
 public:
  /**
   * Input and output format.
   */
  enum InputOutputFormats {
    kStandard = 0,
    kSymmetricForApplyingFourierTransform,
    kSymmetricForPreservingFrequencyResponse,
    kSymmetricForPlottingFrequencyResponse,
    kNumInputOutputFormats
  };

  /**
   * @param[in] fft_length FFT length, @f$L@f$.
   * @param[in] input_format Input format.
   * @param[in] output_format Output format.
   */
  DataSymmetrizing(int fft_length, InputOutputFormats input_format,
                   InputOutputFormats output_format);

  virtual ~DataSymmetrizing() {
  }

  /**
   * @return FFT length.
   */
  int GetFftLength() const {
    return fft_length_;
  }

  /**
   * @return Input format.
   */
  InputOutputFormats GetInputFormat() const {
    return input_format_;
  }

  /**
   * @return Output format.
   */
  InputOutputFormats GetOutputFormat() const {
    return output_format_;
  }

  /**
   * @return Expected input length.
   */
  int GetInputLength() const {
    return input_length_;
  }

  /**
   * @return Expected output length.
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
   * @param[in] data_sequence Input data.
   * @param[out] symmetrized_data_sequence Output data.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& data_sequence,
           std::vector<double>* symmetrized_data_sequence) const;

 private:
  const int fft_length_;
  const InputOutputFormats input_format_;
  const InputOutputFormats output_format_;
  const int input_length_;
  const int output_length_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(DataSymmetrizing);
};

}  // namespace sptk

#endif  // SPTK_UTILS_DATA_SYMMETRIZING_H_
