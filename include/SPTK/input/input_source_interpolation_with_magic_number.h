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

#ifndef SPTK_INPUT_INPUT_SOURCE_INTERPOLATION_WITH_MAGIC_NUMBER_H_
#define SPTK_INPUT_INPUT_SOURCE_INTERPOLATION_WITH_MAGIC_NUMBER_H_

#include <vector>  // std::vector

#include "SPTK/input/input_source_interface.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Interpolate data read from input source.
 */
class InputSourceInterpolationWithMagicNumber : public InputSourceInterface {
 public:
  /**
   * @param[in] frame_period Frame period in samples.
   * @param[in] interpolation_period Interpolation period in samples.
   * @param[in] use_final_frame_for_exceeded_frame If true, reuse the last
   *            frame.
   * @param[in] magic_number Magic number represents discrete symbol.
   * @param[in] source Input source.
   */
  InputSourceInterpolationWithMagicNumber(
      int frame_period, int interpolation_period,
      bool use_final_frame_for_exceeded_frame, double magic_number,
      InputSourceInterface* source);

  virtual ~InputSourceInterpolationWithMagicNumber() {
  }

  /**
   * @return Frame period.
   */
  int GetFramePeriod() const {
    return frame_period_;
  }

  /**
   * @return Interpolation period.
   */
  int GetInterpolationPeriod() const {
    return interpolation_period_;
  }

  /**
   * @return Reuse flag.
   */
  bool UseFinalFrameForExceededFrame() const {
    return use_final_frame_for_exceeded_frame_;
  }

  /**
   * @return Magic number.
   */
  double GetMagicNumber() const {
    return magic_number_;
  }

  /**
   * @return Size of data.
   */
  virtual int GetSize() const {
    return source_ ? source_->GetSize() : 0;
  }

  /**
   * @return True if this object is valid.
   */
  virtual bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[out] buffer Read data.
   * @return True on success, false on failure.
   */
  virtual bool Get(std::vector<double>* buffer);

 private:
  void CalculateIncrement();

  const int frame_period_;
  const int interpolation_period_;
  const int first_interpolation_period_;
  const bool use_final_frame_for_exceeded_frame_;
  const double magic_number_;

  int remained_num_samples_;
  int data_length_;
  int point_index_in_frame_;

  InputSourceInterface* source_;

  bool is_valid_;

  std::vector<double> curr_data_;
  std::vector<double> next_data_;
  std::vector<double> increment_;

  DISALLOW_COPY_AND_ASSIGN(InputSourceInterpolationWithMagicNumber);
};

}  // namespace sptk

#endif  // SPTK_INPUT_INPUT_SOURCE_INTERPOLATION_WITH_MAGIC_NUMBER_H_
