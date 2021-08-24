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

#include "SPTK/compression/inverse_multistage_vector_quantization.h"

#include <algorithm>  // std::fill, std::transform
#include <cstddef>    // std::size_t

namespace sptk {

InverseMultistageVectorQuantization::InverseMultistageVectorQuantization(
    int num_order, int num_stage)
    : num_order_(num_order),
      num_stage_(num_stage),
      inverse_vector_quantization_(num_order_),
      is_valid_(true) {
  if (num_order_ < 0 || num_stage_ <= 0 ||
      !inverse_vector_quantization_.IsValid()) {
    is_valid_ = false;
    return;
  }
}

bool InverseMultistageVectorQuantization::Run(
    const std::vector<int>& codebook_indices,
    const std::vector<std::vector<std::vector<double> > >& codebook_vectors,
    std::vector<double>* reconstructed_vector,
    InverseMultistageVectorQuantization::Buffer* buffer) const {
  // Check inputs.
  if (!is_valid_ ||
      codebook_indices.size() != static_cast<std::size_t>(num_stage_) ||
      codebook_vectors.size() != static_cast<std::size_t>(num_stage_) ||
      NULL == reconstructed_vector || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  const int length(num_order_ + 1);
  if (reconstructed_vector->size() != static_cast<std::size_t>(length)) {
    reconstructed_vector->resize(length);
  }
  if (buffer->quantization_error_.size() != static_cast<std::size_t>(length)) {
    buffer->quantization_error_.resize(length);
  }

  // Initialize reconstructed vector.
  std::fill(reconstructed_vector->begin(), reconstructed_vector->end(), 0.0);

  for (int n(0); n < num_stage_; ++n) {
    if (!inverse_vector_quantization_.Run(codebook_indices[n],
                                          codebook_vectors[n],
                                          &buffer->quantization_error_)) {
      return false;
    }

    std::transform(buffer->quantization_error_.begin(),
                   buffer->quantization_error_.end(),
                   reconstructed_vector->begin(), reconstructed_vector->begin(),
                   [](double e, double x) { return e + x; });
  }

  return true;
}

}  // namespace sptk
