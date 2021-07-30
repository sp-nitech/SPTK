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

#include "SPTK/input/input_source_from_vector.h"

#include <algorithm>  // std::copy, std::fill_n
#include <cstddef>    // std::size_t

namespace sptk {

InputSourceFromVector::InputSourceFromVector(bool zero_padding, int read_size,
                                             std::vector<double>* input_vector)
    : zero_padding_(zero_padding),
      read_size_(read_size),
      vector_size_(
          NULL == input_vector ? 0 : static_cast<int>(input_vector->size())),
      input_vector_(input_vector),
      position_(0),
      is_valid_(true) {
  if (read_size_ <= 0 || NULL == input_vector_) {
    is_valid_ = false;
    return;
  }
}

bool InputSourceFromVector::Get(std::vector<double>* buffer) {
  if (NULL == buffer || !is_valid_ || vector_size_ <= position_) {
    return false;
  }

  const int next_position(position_ + read_size_);
  if (!zero_padding_ && vector_size_ < next_position) {
    return false;
  }

  if (buffer->size() != static_cast<std::size_t>(read_size_)) {
    buffer->resize(read_size_);
  }

  if (next_position <= vector_size_) {
    std::copy(input_vector_->begin() + position_,
              input_vector_->begin() + next_position, buffer->begin());
  } else {
    const int num_nonzeros(vector_size_ - position_);
    const int num_zeros(next_position - vector_size_);
    std::copy(input_vector_->begin() + position_,
              input_vector_->begin() + position_ + num_nonzeros,
              buffer->begin());
    std::fill_n(buffer->begin() + num_nonzeros, num_zeros, 0.0);
  }

  position_ = next_position;

  return true;
}

}  // namespace sptk
