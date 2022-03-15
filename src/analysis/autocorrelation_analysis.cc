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

#include "SPTK/analysis/autocorrelation_analysis.h"

namespace sptk {

AutocorrelationAnalysis::AutocorrelationAnalysis(int frame_length,
                                                 int num_order,
                                                 bool waveform_input)
    : waveform_input_(waveform_input),
      waveform_to_autocorrelation_(NULL),
      spectrum_to_autocorrelation_(NULL),
      is_valid_(true) {
  if (waveform_input_) {
    waveform_to_autocorrelation_ =
        new WaveformToAutocorrelation(frame_length, num_order);
    is_valid_ = waveform_to_autocorrelation_->IsValid();
  } else {
    spectrum_to_autocorrelation_ =
        new SpectrumToAutocorrelation(frame_length, num_order);
    is_valid_ = spectrum_to_autocorrelation_->IsValid();
  }
}

AutocorrelationAnalysis::~AutocorrelationAnalysis() {
  if (waveform_input_) {
    delete waveform_to_autocorrelation_;
  } else {
    delete spectrum_to_autocorrelation_;
  }
}

bool AutocorrelationAnalysis::Run(
    const std::vector<double>& input, std::vector<double>* autocorrelation,
    AutocorrelationAnalysis::Buffer* buffer) const {
  if (!is_valid_ || NULL == autocorrelation || NULL == buffer) {
    return false;
  }

  if (waveform_input_) {
    return waveform_to_autocorrelation_->Run(input, autocorrelation);
  } else {
    return spectrum_to_autocorrelation_->Run(input, autocorrelation,
                                             &buffer->buffer_);
  }
}

}  // namespace sptk
