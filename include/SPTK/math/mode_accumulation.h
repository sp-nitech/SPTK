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

#ifndef SPTK_MATH_MODE_ACCUMULATION_H_
#define SPTK_MATH_MODE_ACCUMULATION_H_

#include <map>            // std::multimap
#include <unordered_map>  // std::unordered_map

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Compute mode given data sequence.
 */
class ModeAccumulation {
 public:
  /**
   * Buffer for ModeAccumulation.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    void Clear() {
      count_.clear();
      maximum_.clear();
    }

    std::unordered_map<double, int> count_;
    std::multimap<int, double> maximum_;

    friend class ModeAccumulation;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_best Number of modes.
   */
  explicit ModeAccumulation(int num_best);

  virtual ~ModeAccumulation() {
  }

  /**
   * @return Number of modes.
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
   * Get @f$n@f$-th mode value and its count.
   *
   * @param[in] rank Rank @f$n@f$.
   * @param[in] buffer Buffer.
   * @param[out] value Mode value.
   * @param[out] count Count of the mode value.
   * @return True on success, false on failure.
   */
  bool GetMode(int rank, const ModeAccumulation::Buffer& buffer, double* value,
               int* count) const;

  /**
   * Clear buffer.
   *
   * @param[out] buffer Buffer.
   */
  void Clear(ModeAccumulation::Buffer* buffer) const;

  /**
   * Accumulate mode.
   *
   * @param[in] data Input data.
   * @param[in,out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(double data, ModeAccumulation::Buffer* buffer) const;

 private:
  const int num_best_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(ModeAccumulation);
};

}  // namespace sptk

#endif  // SPTK_MATH_MODE_ACCUMULATION_H_
