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

#include "SPTK/compression/vector_quantization.h"

#include <cfloat>   // DBL_MAX
#include <cstddef>  // std::size_t

namespace sptk {

VectorQuantization::VectorQuantization(int num_order)
    : num_order_(num_order),
      distance_calculation_(
          num_order_, DistanceCalculation::DistanceMetrics::kSquaredEuclidean),
      is_valid_(true) {
  if (num_order_ < 0 || !distance_calculation_.IsValid()) {
    is_valid_ = false;
    return;
  }
}

bool VectorQuantization::Run(
    const std::vector<double>& input_vector,
    const std::vector<std::vector<double> >& codebook_vectors,
    int* codebook_index) const {
  // Check inputs.
  const int codebook_size(static_cast<int>(codebook_vectors.size()));
  if (!is_valid_ ||
      input_vector.size() != static_cast<std::size_t>(num_order_ + 1) ||
      0 == codebook_size || NULL == codebook_index) {
    return false;
  }

  int index(0);
  double min_distance(DBL_MAX);

  for (int i(0); i < codebook_size; ++i) {
    double distance;
    if (!distance_calculation_.Run(input_vector, codebook_vectors[i],
                                   &distance)) {
      return false;
    }
    if (distance < min_distance) {
      index = i;
      min_distance = distance;
    }
  }

  *codebook_index = index;

  return true;
}

}  // namespace sptk
