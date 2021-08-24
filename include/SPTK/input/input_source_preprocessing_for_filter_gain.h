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

#ifndef SPTK_INPUT_INPUT_SOURCE_PREPROCESSING_FOR_FILTER_GAIN_H_
#define SPTK_INPUT_INPUT_SOURCE_PREPROCESSING_FOR_FILTER_GAIN_H_

#include <vector>  // std::vector

#include "SPTK/input/input_source_interface.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Preprocess filter gain.
 */
class InputSourcePreprocessingForFilterGain : public InputSourceInterface {
 public:
  /**
   * Type of filter gain.
   */
  enum FilterGainType {
    kLinear = 0,
    kLog,
    kUnity,
    kUnityForAllZeroFilter,
  };

  /**
   * @param[in] gain_type Gain type.
   * @param[in] source Input source.
   */
  InputSourcePreprocessingForFilterGain(FilterGainType gain_type,
                                        InputSourceInterface* source);

  virtual ~InputSourcePreprocessingForFilterGain() {
  }

  /**
   * @return Gain type.
   */
  FilterGainType GetFilterGainType() const {
    return gain_type_;
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
  const FilterGainType gain_type_;
  InputSourceInterface* source_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(InputSourcePreprocessingForFilterGain);
};

}  // namespace sptk

#endif  // SPTK_INPUT_INPUT_SOURCE_PREPROCESSING_FOR_FILTER_GAIN_H_
