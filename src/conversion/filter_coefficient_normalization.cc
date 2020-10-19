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

#include "SPTK/conversion/filter_coefficient_normalization.h"

#include <algorithm>   // std::transform
#include <cstddef>     // std::size_t
#include <functional>  // std::bind1st, std::multiplies

namespace sptk {

FilterCoefficientNormalization::FilterCoefficientNormalization(int num_order)
    : num_order_(num_order), is_valid_(true) {
  if (num_order_ < 0) {
    is_valid_ = false;
  }
}

bool FilterCoefficientNormalization::Run(
    const std::vector<double>& filter_coefficients,
    std::vector<double>* normalized_filter_coefficients) const {
  // check inputs
  if (!is_valid_ ||
      filter_coefficients.size() != static_cast<std::size_t>(num_order_ + 1) ||
      0.0 == filter_coefficients[0] || NULL == normalized_filter_coefficients) {
    return false;
  }

  // prepare memory
  if (normalized_filter_coefficients->size() !=
      static_cast<std::size_t>(num_order_ + 1)) {
    normalized_filter_coefficients->resize(num_order_ + 1);
  }

  // normarize
  (*normalized_filter_coefficients)[0] = 1.0 / filter_coefficients[0];

  std::transform(filter_coefficients.begin() + 1, filter_coefficients.end(),
                 normalized_filter_coefficients->begin() + 1,
                 std::bind1st(std::multiplies<double>(),
                              (*normalized_filter_coefficients)[0]));

  return true;
}

}  // namespace sptk
