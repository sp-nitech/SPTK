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
