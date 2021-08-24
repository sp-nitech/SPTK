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

#ifndef SPTK_MATH_MINMAX_ACCUMULATION_H_
#define SPTK_MATH_MINMAX_ACCUMULATION_H_

#include <list>     // std::list
#include <utility>  // std::pair

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Compute minimum and maximum given data sequence.
 */
class MinMaxAccumulation {
 public:
  /**
   * Buffer for MinMaxAccumulation.
   */
  class Buffer {
   public:
    Buffer() : position_(0) {
    }

    virtual ~Buffer() {
    }

   private:
    void Clear() {
      position_ = 0;
      minimum_.clear();
      maximum_.clear();
    }

    int position_;
    std::list<std::pair<int, double> > minimum_;
    std::list<std::pair<int, double> > maximum_;

    friend class MinMaxAccumulation;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_best Number of minimum/maximum numbers.
   */
  explicit MinMaxAccumulation(int num_best);

  virtual ~MinMaxAccumulation() {
  }

  /**
   * @return Number of minimum/maximum numbers.
   */
  int GetNumBest() const {
    return num_best_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * Get @f$n@f$-th minimum value and its position.
   *
   * @param[in] buffer Buffer.
   * @param[in] rank Rank @f$n@f$.
   * @param[out] position Position of minimum value.
   * @param[out] value Minimum value.
   * @return True on success, false on failure.
   */
  bool GetMinimum(const MinMaxAccumulation::Buffer& buffer, int rank,
                  int* position, double* value) const;

  /**
   * Get @f$n@f$-th maximum value and its position.
   *
   * @param[in] buffer Buffer.
   * @param[in] rank Rank @f$n@f$.
   * @param[out] position Position of maximum value.
   * @param[out] value Maximum value.
   * @return True on success, false on failure.
   */
  bool GetMaximum(const MinMaxAccumulation::Buffer& buffer, int rank,
                  int* position, double* value) const;

  /**
   * Clear buffer.
   *
   * @param[out] buffer Buffer.
   */
  void Clear(MinMaxAccumulation::Buffer* buffer) const;

  /**
   * Accumulate minimum and maximum.
   *
   * @param[in] data Input data.
   * @param[in,out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(double data, MinMaxAccumulation::Buffer* buffer) const;

 private:
  const int num_best_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(MinMaxAccumulation);
};

}  // namespace sptk

#endif  // SPTK_MATH_MINMAX_ACCUMULATION_H_
