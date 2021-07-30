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

#include "SPTK/filter/inverse_pseudo_quadrature_mirror_filter_banks.h"

#include <cstddef>  // std::size_t

#include "SPTK/utils/misc_utils.h"

namespace sptk {

InversePseudoQuadratureMirrorFilterBanks::
    InversePseudoQuadratureMirrorFilterBanks(int num_subband,
                                             int num_filter_order,
                                             double attenuation,
                                             int num_iteration,
                                             double convergence_threshold,
                                             double initial_step_size)
    : num_subband_(num_subband),
      all_zero_filter_(num_filter_order, false),
      is_valid_(true) {
  if (!all_zero_filter_.IsValid()) {
    is_valid_ = false;
    return;
  }

  if (!MakePseudoQuadratureMirrorFilterBanks(
          true, num_subband_, num_filter_order, attenuation, num_iteration,
          convergence_threshold, initial_step_size, &filter_banks_,
          &is_converged_)) {
    is_valid_ = false;
    return;
  }
}

bool InversePseudoQuadratureMirrorFilterBanks::Run(
    const std::vector<double>& input, double* output,
    InversePseudoQuadratureMirrorFilterBanks::Buffer* buffer) const {
  // Check inputs.
  if (!is_valid_ || input.size() != static_cast<std::size_t>(num_subband_) ||
      output == nullptr || buffer == nullptr)
    return false;

  // Prepare memories.
  if (buffer->buffer_for_all_zero_filter_.size() !=
      static_cast<std::size_t>(num_subband_)) {
    for (std::vector<AllZeroDigitalFilter::Buffer*>::iterator itr(
             buffer->buffer_for_all_zero_filter_.begin());
         itr != buffer->buffer_for_all_zero_filter_.end(); ++itr) {
      delete (*itr);
      *itr = NULL;
    }

    buffer->buffer_for_all_zero_filter_.resize(num_subband_);
    for (std::vector<AllZeroDigitalFilter::Buffer*>::iterator itr(
             buffer->buffer_for_all_zero_filter_.begin());
         itr != buffer->buffer_for_all_zero_filter_.end(); ++itr) {
      *itr = new AllZeroDigitalFilter::Buffer();
    }
  }

  double sum(0.0);
  for (int k(0); k < num_subband_; ++k) {
    double tmp;
    if (!all_zero_filter_.Run(filter_banks_[k], input[k], &tmp,
                              buffer->buffer_for_all_zero_filter_[k])) {
      return false;
    }
    sum += tmp;
  }
  *output = sum;

  return true;
}

}  // namespace sptk
