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

#ifndef SPTK_ANALYSIS_ZERO_CROSSING_ANALYSIS_H_
#define SPTK_ANALYSIS_ZERO_CROSSING_ANALYSIS_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Perform zero-crossing analysis.
 *
 * The input is the @f$L@f$-length waveform signals:
 * @f[
 *   \begin{array}{cccc}
 *     x(0), & x(1), & \ldots, & x(L-1),
 *   \end{array}
 * @f]
 * and the output is the number of zero-crossings calculated by
 * @f[
 *   \frac{1}{2} \sum_{l=0}^{L-1} |\mathrm{sgn}(x(l)) - \mathrm{sgn}(x(l-1))|.
 * @f]
 * where @f$\mathrm{sgn}(\cdot)@f$ returns -1 if the input is negative,
 * otherwise returns 1. Note that @f$x(-1)@f$ is taken from the previous frame.
 */
class ZeroCrossingAnalysis {
 public:
  /**
   * Buffer for ZeroCrossingAnalysis class.
   */
  class Buffer {
   public:
    Buffer() : is_first_frame_(true) {
    }

    virtual ~Buffer() {
    }

   private:
    bool is_first_frame_;
    double latest_signal_;

    friend class ZeroCrossingAnalysis;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] frame_length Frame length in points, @f$L@f$.
   */
  explicit ZeroCrossingAnalysis(int frame_length);

  virtual ~ZeroCrossingAnalysis() {
  }

  /**
   * @return Frame length.
   */
  int GetFrameLength() const {
    return frame_length_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] signals @f$L@f$-length waveform signals.
   * @param[out] num_zero_crossing Number of zero crossings.
   * @param[in,out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& signals, int* num_zero_crossing,
           ZeroCrossingAnalysis::Buffer* buffer) const;

 private:
  const int frame_length_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(ZeroCrossingAnalysis);
};

}  // namespace sptk

#endif  // SPTK_ANALYSIS_ZERO_CROSSING_ANALYSIS_H_
