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

#ifndef SPTK_MATH_HISTOGRAM_CALCULATION_H_
#define SPTK_MATH_HISTOGRAM_CALCULATION_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Calculate histogram.
 *
 * The input is a data sequence:
 * @f[
 *   \begin{array}{ccc}
 *     x(0), & x(1), & \ldots,
 *   \end{array}
 * @f]
 * and the output is the histogram of data:
 * @f[
 *   \begin{array}{cccc}
 *     b(1), & b(2), & \ldots, & b(N),
 *   \end{array}
 * @f]
 * where @f$N@f$ is the number of bins.
 *
 * The width of the bin is calculated as
 * @f[
 *    \frac{y_U - y_L}{N},
 * @f]
 * where @f$y_U@f$ and @f$y_L@f$ are the upper bound and the lower bound of
 * the histogram. The data that satisfies @f$x(t) > y_U@f$ or
 * @f$x(t) < y_L@f$ is not contributed to any bins.
 */
class HistogramCalculation {
 public:
  /**
   * @param[in] num_bin Number of bins, @f$N@f$.
   * @param[in] lower_bound Lower bound, @f$y_L@f$.
   * @param[in] upper_bound Upper bound, @f$y_U@f$.
   */
  HistogramCalculation(int num_bin, double lower_bound, double upper_bound);

  virtual ~HistogramCalculation() {
  }

  /**
   * @return Number of bins.
   */
  int GetNumBin() const {
    return num_bin_;
  }

  /**
   * @return Lower bound.
   */
  double GetLowerBound() const {
    return lower_bound_;
  }

  /**
   * @return Upper bound.
   */
  double GetUpperBound() const {
    return upper_bound_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] data Input data.
   * @param[out] histogram Histogram.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& data,
           std::vector<double>* histogram) const;

 private:
  const int num_bin_;
  const double lower_bound_;
  const double upper_bound_;
  const double bin_width_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(HistogramCalculation);
};

}  // namespace sptk

#endif  // SPTK_MATH_HISTOGRAM_CALCULATION_H_
