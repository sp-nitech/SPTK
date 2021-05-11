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

#ifndef SPTK_MATH_STATISTICS_ACCUMULATION_H_
#define SPTK_MATH_STATISTICS_ACCUMULATION_H_

#include <algorithm>  // std::fill
#include <vector>     // std::vector

#include "SPTK/math/symmetric_matrix.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Accumulate statistics.
 *
 * The input of is an @f$M@f$-th order vector:
 * @f[
 *   \begin{array}{cccc}
 *     x_t(0), & x_t(1), & \ldots, & x_t(M).
 *   \end{array}
 * @f]
 * After running @c Run @f$T@f$ times, the following statistics are obtained:
 * @f{eqnarray}{
 *        S_0 &=& T, \\
 *     S_1(m) &=& \sum_{t=0}^{T-1} x_t(m), \\
 *   S_2(m,n) &=& \sum_{t=0}^{T-1} x_t(m) x_t(n).
 * @f}
 * Then, the moments, e.g., mean and covariance, of the input data can be
 * computed from the accumulated statistics @f$\{S_k\}_{k=0}^K@f$.
 */
class StatisticsAccumulation {
 public:
  /**
   * Buffer for StatisticsAccumulation class.
   */
  class Buffer {
   public:
    Buffer() : zeroth_order_statistics_(0) {
    }

    virtual ~Buffer() {
    }

   private:
    void Clear() {
      zeroth_order_statistics_ = 0;
      std::fill(first_order_statistics_.begin(), first_order_statistics_.end(),
                0.0);
      second_order_statistics_.Fill(0.0);
    }

    int zeroth_order_statistics_;
    std::vector<double> first_order_statistics_;
    SymmetricMatrix second_order_statistics_;

    friend class StatisticsAccumulation;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_order Order of vector, @f$M@f$.
   * @param[in] num_statistics_order Order of statistics, @f$K@f$.
   */
  StatisticsAccumulation(int num_order, int num_statistics_order);

  virtual ~StatisticsAccumulation() {
  }

  /**
   * @return Order of vector.
   */
  int GetNumOrder() const {
    return num_order_;
  }

  /**
   * @return Order of statistics.
   */
  int GetNumStatisticsOrder() const {
    return num_statistics_order_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] buffer Buffer.
   * @param[out] num_data Number of accumulated data.
   * @return True on success, false on failure.
   */
  bool GetNumData(const StatisticsAccumulation::Buffer& buffer,
                  int* num_data) const;

  /**
   * @param[in] buffer Buffer.
   * @param[out] sum Summation of accumulated data.
   * @return True on success, false on failure.
   */
  bool GetSum(const StatisticsAccumulation::Buffer& buffer,
              std::vector<double>* sum) const;

  /**
   * @param[in] buffer Buffer.
   * @param[out] mean Mean of accumulated data.
   * @return True on success, false on failure.
   */
  bool GetMean(const StatisticsAccumulation::Buffer& buffer,
               std::vector<double>* mean) const;

  /**
   * @param[in] buffer Buffer.
   * @param[out] variance Diagonal covariance of accumulated data.
   * @return True on success, false on failure.
   */
  bool GetDiagonalCovariance(const StatisticsAccumulation::Buffer& buffer,
                             std::vector<double>* variance) const;

  /**
   * @param[in] buffer Buffer.
   * @param[out] standard_deviation Standard deviation of accumulated data.
   * @return True on success, false on failure.
   */
  bool GetStandardDeviation(const StatisticsAccumulation::Buffer& buffer,
                            std::vector<double>* standard_deviation) const;

  /**
   * @param[in] buffer Buffer.
   * @param[out] full_covariance Full covariance of accumulated data.
   * @return True on success, false on failure.
   */
  bool GetFullCovariance(const StatisticsAccumulation::Buffer& buffer,
                         SymmetricMatrix* full_covariance) const;

  /**
   * @param[in] buffer Buffer.
   * @param[out] correlation Correlation of accumulated data.
   * @return True on success, false on failure.
   */
  bool GetCorrelation(const StatisticsAccumulation::Buffer& buffer,
                      SymmetricMatrix* correlation) const;

  /**
   * Clear buffer.
   *
   * @param[in,out] buffer Buffer.
   */
  void Clear(StatisticsAccumulation::Buffer* buffer) const;

  /**
   * Accumulate statistics.
   *
   * @param[in] data @f$M@f$-th order input vector.
   * @param[in,out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& data,
           StatisticsAccumulation::Buffer* buffer) const;

 private:
  const int num_order_;
  const int num_statistics_order_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(StatisticsAccumulation);
};

}  // namespace sptk

#endif  // SPTK_MATH_STATISTICS_ACCUMULATION_H_
