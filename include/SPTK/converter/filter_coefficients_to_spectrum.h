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

#ifndef SPTK_CONVERTER_FILTER_COEFFICIENTS_TO_SPECTRUM_H_
#define SPTK_CONVERTER_FILTER_COEFFICIENTS_TO_SPECTRUM_H_

#include <vector>  // std::vector

#include "SPTK/math/real_valued_fast_fourier_transform.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

class FilterCoefficientsToSpectrum {
 public:
  //
  enum OutputFormats {
    kLogAmplitudeSpectrumInDecibels = 0,
    kLogAmplitudeSpectrum,
    kAmplitudeSpectrum,
    kPowerSpectrum,
    kNumOutputFormats
  };

  class Buffer {
   public:
    Buffer() {
    }
    virtual ~Buffer() {
    }

   private:
    RealValuedFastFourierTransform::Buffer fast_fourier_transform_buffer_;
    std::vector<double> fast_fourier_transform_input_;
    std::vector<double> fast_fourier_transform_real_output_;
    std::vector<double> fast_fourier_transform_imaginary_output_;
    std::vector<double> numerator_of_transfer_function_;
    std::vector<double> denominator_of_transfer_function_;
    friend class FilterCoefficientsToSpectrum;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  //
  FilterCoefficientsToSpectrum(int num_numerator_order,
                               int num_denominator_order, int fft_length,
                               OutputFormats output_format,
                               double epsilon_for_calculating_logarithms,
                               double relative_floor_in_decibels);

  //
  virtual ~FilterCoefficientsToSpectrum() {
  }

  //
  int GetNumNumeratorOrder() const {
    return num_numerator_order_;
  }

  //
  int GetNumDenominatorOrder() const {
    return num_denominator_order_;
  }

  //
  int GetFftLength() const {
    return fft_length_;
  }

  //
  OutputFormats GetOutputFormat() const {
    return output_format_;
  }

  //
  double GetEpsilonForCalculatingLogarithms() const {
    return epsilon_for_calculating_logarithms_;
  }

  //
  double GetRelativeFloorInDecibels() const {
    return relative_floor_in_decibels_;
  }

  //
  bool IsValid() const {
    return is_valid_;
  }

  //
  bool Run(const std::vector<double>& numerator_coefficients,
           const std::vector<double>& denominator_coefficients,
           std::vector<double>* spectrum,
           FilterCoefficientsToSpectrum::Buffer* buffer) const;

 private:
  //
  const int num_numerator_order_;

  //
  const int num_denominator_order_;

  //
  const int fft_length_;

  //
  const OutputFormats output_format_;

  //
  const double epsilon_for_calculating_logarithms_;

  //
  const double relative_floor_in_decibels_;

  //
  const RealValuedFastFourierTransform fast_fourier_transform_;

  //
  bool is_valid_;

  //
  DISALLOW_COPY_AND_ASSIGN(FilterCoefficientsToSpectrum);
};

}  // namespace sptk

#endif  // SPTK_CONVERTER_FILTER_COEFFICIENTS_TO_SPECTRUM_H_
