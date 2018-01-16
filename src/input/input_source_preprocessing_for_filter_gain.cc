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
//                1996-2018  Nagoya Institute of Technology          //
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

#include "SPTK/input/input_source_preprocessing_for_filter_gain.h"

#include <algorithm>   // std::transform
#include <cmath>       // std::exp
#include <functional>  // std::bind1st, std::multiplies

namespace sptk {

bool InputSourcePreprocessingForFilterGain::Get(std::vector<double>* buffer) {
  if (NULL == buffer || !is_valid_) {
    return false;
  }

  if (!source_->Get(buffer)) {
    return false;
  }

  switch (gain_type_) {
    case InputSourcePreprocessingForFilterGain::FilterGainType::kLinear: {
      break;
    }
    case InputSourcePreprocessingForFilterGain::FilterGainType::kLog: {
      (*buffer)[0] = std::exp((*buffer)[0]);
      break;
    }
    case InputSourcePreprocessingForFilterGain::FilterGainType::kUnity: {
      (*buffer)[0] = 1.0;
      break;
    }
    case InputSourcePreprocessingForFilterGain::FilterGainType::
        kUnityForAllZeroFilter: {
      if (0.0 == (*buffer)[0]) return false;
      const double inverse_of_b0(1.0 / (*buffer)[0]);
      std::transform(buffer->begin(), buffer->end(), buffer->begin(),
                     std::bind1st(std::multiplies<double>(), inverse_of_b0));
      break;
    }
    default: { return false; }
  }

  return true;
}

}  // namespace sptk
