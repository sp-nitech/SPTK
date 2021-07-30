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

#include "SPTK/compression/multistage_vector_quantization.h"

#include <algorithm>  // std::copy, std::transform
#include <cstddef>    // std::size_t

namespace sptk {

MultistageVectorQuantization::MultistageVectorQuantization(int num_order,
                                                           int num_stage)
    : num_order_(num_order),
      num_stage_(num_stage),
      vector_quantization_(num_order_),
      is_valid_(true) {
  if (num_order_ < 0 || num_stage_ <= 0 || !vector_quantization_.IsValid()) {
    is_valid_ = false;
    return;
  }
}

bool MultistageVectorQuantization::Run(
    const std::vector<double>& input_vector,
    const std::vector<std::vector<std::vector<double> > >& codebook_vectors,
    std::vector<int>* codebook_indices,
    MultistageVectorQuantization::Buffer* buffer) const {
  // Check inputs.
  const int length(num_order_ + 1);
  if (!is_valid_ || input_vector.size() != static_cast<std::size_t>(length) ||
      codebook_vectors.size() != static_cast<std::size_t>(num_stage_) ||
      NULL == codebook_indices || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  if (codebook_indices->size() != static_cast<std::size_t>(num_stage_)) {
    codebook_indices->resize(num_stage_);
  }
  if (buffer->quantization_error_.size() != static_cast<std::size_t>(length)) {
    buffer->quantization_error_.resize(length);
  }

  // Initialize quantization error.
  std::copy(input_vector.begin(), input_vector.end(),
            buffer->quantization_error_.begin());

  for (int n(0); n < num_stage_; ++n) {
    if (!vector_quantization_.Run(buffer->quantization_error_,
                                  codebook_vectors[n],
                                  &((*codebook_indices)[n]))) {
      return false;
    }

    if (n < num_stage_ - 1) {
      std::transform(buffer->quantization_error_.begin(),
                     buffer->quantization_error_.end(),
                     codebook_vectors[n][(*codebook_indices)[n]].begin(),
                     buffer->quantization_error_.begin(),
                     [](double e, double c) { return e - c; });
    }
  }

  return true;
}

}  // namespace sptk
