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
//                1996-2019  Nagoya Institute of Technology          //
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

#include "SPTK/quantizer/vector_quantization.h"

#include <cfloat>   // DBL_MAX
#include <cstddef>  // std::size_t

namespace sptk {

VectorQuantization::VectorQuantization(int num_order)
    : num_order_(num_order),
      distance_calculator_(
          num_order_, DistanceCalculator::DistanceMetrics::kSquaredEuclidean),
      is_valid_(true) {
  if (num_order_ < 0 || !distance_calculator_.IsValid()) {
    is_valid_ = false;
  }
}

bool VectorQuantization::Run(
    const std::vector<double>& input_vector,
    const std::vector<std::vector<double> >& codebook_vectors,
    int* codebook_index) const {
  if (!is_valid_ ||
      input_vector.size() != static_cast<std::size_t>(num_order_ + 1) ||
      codebook_vectors.empty() || NULL == codebook_index) {
    return false;
  }

  const int codebook_size(codebook_vectors.size());
  int index(0);
  double minimum_distance(DBL_MAX);

  for (int i(0); i < codebook_size; ++i) {
    double distance;
    if (!distance_calculator_.Run(input_vector, codebook_vectors[i],
                                  &distance)) {
      return false;
    }
    if (distance < minimum_distance) {
      index = i;
      minimum_distance = distance;
    }
  }

  *codebook_index = index;

  return true;
}

}  // namespace sptk
