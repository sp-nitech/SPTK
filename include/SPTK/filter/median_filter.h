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

#ifndef SPTK_FILTER_MEDIAN_FILTER_H_
#define SPTK_FILTER_MEDIAN_FILTER_H_

#include <deque>   // std::deque
#include <vector>  // std::vector

#include "SPTK/input/input_source_interface.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Apply a median filter to signals.
 */
class MedianFilter : public InputSourceInterface {
 public:
  /**
   * @param[in] num_input_order Order of input, @f$M@f$.
   * @param[in] num_filter_order Order of filter.
   * @param[in] input_source Input data sequence.
   * @param[in] apply_each_dimension Whether to apply filter to each dimension.
   * @param[in] use_magic_number Whether to use a magic number.
   * @param[in] magic_number A magic number.
   */
  MedianFilter(int num_input_order, int num_filter_order,
               InputSourceInterface* input_source, bool apply_each_dimension,
               bool use_magic_number, double magic_number = 0.0);

  virtual ~MedianFilter() {
  }

  /**
   * @return Order of input.
   */
  int GetNumInputOrder() const {
    return num_input_order_;
  }

  /**
   * @return Order of median filter.
   */
  int GetNumFilterOrder() const {
    return num_filter_order_;
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
    return apply_each_dimension_ ? num_input_order_ + 1 : 1;
  }

  /**
   * @return True if this object is valid.
   */
  virtual bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[out] output Filtered input.
   * @return True on success, false on failure.
   */
  virtual bool Get(std::vector<double>* output);

 private:
  bool Forward();

  const int num_input_order_;
  const int num_filter_order_;
  InputSourceInterface* input_source_;
  const bool apply_each_dimension_;
  const bool use_magic_number_;
  const double magic_number_;

  bool is_valid_;

  std::vector<double> buffer_;
  std::vector<double> flat_;
  std::deque<std::vector<double> > queue_;
  int count_down_;

  DISALLOW_COPY_AND_ASSIGN(MedianFilter);
};

}  // namespace sptk

#endif  // SPTK_FILTER_MEDIAN_FILTER_H_
