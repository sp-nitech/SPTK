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

#include "SPTK/conversion/waveform_to_spectrum.h"

namespace sptk {

WaveformToSpectrum::WaveformToSpectrum(
    int frame_length, int fft_length,
    SpectrumToSpectrum::InputOutputFormats output_format, double epsilon,
    double relative_floor_in_decibels)
    : filter_coefficients_to_spectrum_(frame_length - 1, 0, fft_length,
                                       output_format, epsilon,
                                       relative_floor_in_decibels),
      dummy_for_filter_coefficients_to_spectrum_(1, 1.0) {
}

bool WaveformToSpectrum::Run(const std::vector<double>& waveform,
                             std::vector<double>* spectrum,
                             WaveformToSpectrum::Buffer* buffer) const {
  return filter_coefficients_to_spectrum_.Run(
      waveform, dummy_for_filter_coefficients_to_spectrum_, spectrum,
      &buffer->buffer_);
}

}  // namespace sptk
