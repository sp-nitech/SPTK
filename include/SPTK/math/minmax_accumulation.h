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
//                1996-2021  Nagoya Institute of Technology          //
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
