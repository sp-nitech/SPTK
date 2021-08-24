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

#include "SPTK/compression/inverse_vector_quantization.h"

#include <algorithm>  // std::copy
#include <cstddef>    // std::size_t

namespace sptk {

InverseVectorQuantization::InverseVectorQuantization(int num_order)
    : num_order_(num_order), is_valid_(true) {
  if (num_order_ < 0) {
    is_valid_ = false;
    return;
  }
}

bool InverseVectorQuantization::Run(
    int codebook_index,
    const std::vector<std::vector<double> >& codebook_vectors,
    std::vector<double>* reconstructed_vector) const {
  // Check inputs.
  const int codebook_size(static_cast<int>(codebook_vectors.size()));
  if (!is_valid_ || codebook_index < 0 || codebook_size <= codebook_index ||
      NULL == reconstructed_vector) {
    return false;
  }

  const int length(num_order_ + 1);
  if (codebook_vectors[codebook_index].size() !=
      static_cast<std::size_t>(length)) {
    return false;
  }

  // Prepare memories.
  if (reconstructed_vector->size() != static_cast<std::size_t>(length)) {
    reconstructed_vector->resize(length);
  }

  std::copy(codebook_vectors[codebook_index].begin(),
            codebook_vectors[codebook_index].end(),
            reconstructed_vector->begin());

  return true;
}

}  // namespace sptk
