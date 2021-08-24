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
