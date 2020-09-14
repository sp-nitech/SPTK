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
//                1996-2020  Nagoya Institute of Technology          //
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

#include "SPTK/analyzer/adaptive_mel_generalized_cepstral_analysis.h"

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
