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
   * @param[out] unbiased_covariance Unbiased covariance of accumulated data.
   * @return True on success, false on failure.
   */
  bool GetUnbiasedCovariance(const StatisticsAccumulation::Buffer& buffer,
                             SymmetricMatrix* unbiased_covariance) const;

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
