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
