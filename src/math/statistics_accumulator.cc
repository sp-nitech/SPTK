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
//                1996-2017  Nagoya Institute of Technology          //
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

#include "SPTK/math/statistics_accumulator.h"

#include <algorithm>   // std::copy, std::fill, std::transform
#include <cstddef>     // std::size_t
#include <functional>  // std::bind1st, std::plus

namespace sptk {

StatisticsAccumulator::StatisticsAccumulator(int num_order,
                                             int num_statistics_order)
    : num_order_(num_order),
      num_statistics_order_(num_statistics_order),
      is_valid_(true) {
  if (num_order_ < 0 || num_statistics_order_ < 0 ||
      2 < num_statistics_order_) {
    is_valid_ = false;
  }
}

void StatisticsAccumulator::Clear(StatisticsAccumulator::Buffer* buffer) const {
  if (NULL != buffer) buffer->Clear();
}

bool StatisticsAccumulator::GetSum(const StatisticsAccumulator::Buffer& buffer,
                                   std::vector<double>* sum) const {
  if (num_statistics_order_ < 1 || buffer.zeroth_order_statistics_ <= 0 ||
      NULL == sum) {
    return false;
  }

  if (sum->size() < static_cast<std::size_t>(num_order_ + 1)) {
    sum->resize(num_order_ + 1);
  }

  std::copy(buffer.first_order_statistics_.begin(),
            buffer.first_order_statistics_.end(), sum->begin());

  return true;
}

bool StatisticsAccumulator::GetMean(const StatisticsAccumulator::Buffer& buffer,
                                    std::vector<double>* mean) const {
  if (num_statistics_order_ < 1 || buffer.zeroth_order_statistics_ <= 0 ||
      NULL == mean) {
    return false;
  }

  if (mean->size() < static_cast<std::size_t>(num_order_ + 1)) {
    mean->resize(num_order_ + 1);
  }

  std::transform(buffer.first_order_statistics_.begin(),
                 buffer.first_order_statistics_.end(), mean->begin(),
                 std::bind1st(std::multiplies<double>(),
                              1.0 / buffer.zeroth_order_statistics_));

  return true;
}

bool StatisticsAccumulator::Run(const std::vector<double>& data,
                                StatisticsAccumulator::Buffer* buffer) const {
  // check inputs
  const int length(num_order_ + 1);
  if (!is_valid_ || data.size() != static_cast<std::size_t>(length) ||
      NULL == buffer) {
    return false;
  }

  // prepare buffer
  if (1 <= num_statistics_order_) {
    if (buffer->first_order_statistics_.size() <
        static_cast<std::size_t>(length)) {
      buffer->first_order_statistics_.resize(length);
      std::fill(buffer->first_order_statistics_.begin(),
                buffer->first_order_statistics_.end(), 0.0);
      buffer->zeroth_order_statistics_ = 0.0;
    }
  }
  if (2 <= num_statistics_order_) {
    // TODO(takenori): prepare triangle matrix and initialize it
  }

  // 0th order
  ++(buffer->zeroth_order_statistics_);

  // 1st order
  if (1 <= num_statistics_order_) {
    std::transform(
        data.begin(), data.end(), buffer->first_order_statistics_.begin(),
        buffer->first_order_statistics_.begin(), std::plus<double>());
  }

  // 2nd order
  if (2 <= num_statistics_order_) {
    // TODO(takenori): accumulate second-order statistics
  }

  return true;
}

}  // namespace sptk
