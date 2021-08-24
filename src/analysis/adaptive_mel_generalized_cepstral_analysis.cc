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

#include "SPTK/analysis/adaptive_mel_generalized_cepstral_analysis.h"

namespace sptk {

AdaptiveMelGeneralizedCepstralAnalysis::AdaptiveMelGeneralizedCepstralAnalysis(
    int num_order, int num_pade_order, int num_stage, double alpha,
    double min_epsilon, double momentum, double forgetting_factor,
    double step_size_factor)
    : generalized_cepstral_analysis_(num_order, num_stage, min_epsilon,
                                     momentum, forgetting_factor,
                                     step_size_factor),
      mel_cepstral_analysis_(num_order, num_pade_order, alpha, min_epsilon,
                             momentum, forgetting_factor, step_size_factor),
      is_valid_(true) {
  if ((0 != num_stage && 0.0 != alpha) ||
      (0 != num_stage && !generalized_cepstral_analysis_.IsValid()) ||
      (0 == num_stage && !mel_cepstral_analysis_.IsValid())) {
    is_valid_ = false;
    return;
  }
}

bool AdaptiveMelGeneralizedCepstralAnalysis::Run(
    double input_signal, double* prediction_error,
    std::vector<double>* mel_generalized_cepstrum,
    AdaptiveMelGeneralizedCepstralAnalysis::Buffer* buffer) const {
  // Check inputs.
  if (!is_valid_ || NULL == prediction_error ||
      NULL == mel_generalized_cepstrum || NULL == buffer) {
    return false;
  }

  if (0 == generalized_cepstral_analysis_.GetNumStage()) {
    return mel_cepstral_analysis_.Run(
        input_signal, prediction_error, mel_generalized_cepstrum,
        &buffer->buffer_for_mel_cepstral_analysis_);
  }

  return generalized_cepstral_analysis_.Run(
      input_signal, prediction_error, mel_generalized_cepstrum,
      &buffer->buffer_for_generalized_cepstral_analysis_);
}

}  // namespace sptk
