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
//                1996-2017  Nagoya Institute of Technology          //
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

#include "SPTK/utils/data_symmetrizing.h"

#include <algorithm>  // std::copy
#include <cstddef>    // std::size_t

namespace {

int CalculateDataLength(int fft_length,
                        sptk::DataSymmetrizing::InputOutputFormats format) {
  switch (format) {
    case sptk::DataSymmetrizing::InputOutputFormats::kStandard: {
      return fft_length / 2 + 1;
    }
    case sptk::DataSymmetrizing::InputOutputFormats::
        kSymmetricForApplyingFourierTransform: {
      return fft_length;
    }
    case sptk::DataSymmetrizing::InputOutputFormats::
        kSymmetricForFormingImpulseResponse: {
      return fft_length + 1;
    }
    case sptk::DataSymmetrizing::InputOutputFormats::
        kSymmetricForPlottingImpulseResponse: {
      return fft_length + 1;
    }
    default: { return 0; }
  }
}

}  // namespace

namespace sptk {

DataSymmetrizing::DataSymmetrizing(int fft_length,
                                   InputOutputFormats input_format,
                                   InputOutputFormats output_format)
    : fft_length_(fft_length),
      input_format_(input_format),
      output_format_(output_format),
      input_length_(CalculateDataLength(fft_length_, input_format_)),
      output_length_(CalculateDataLength(fft_length_, output_format_)),
      is_valid_(true) {
  if (fft_length_ < 4 || 0 == input_length_ || 0 == output_length_) {
    is_valid_ = false;
  }
}

bool DataSymmetrizing::Run(
    const std::vector<double>& data_sequence,
    std::vector<double>* symmetrized_data_sequence) const {
  // check inputs
  if (!is_valid_ ||
      data_sequence.size() != static_cast<std::size_t>(input_length_) ||
      NULL == symmetrized_data_sequence) {
    return false;
  }

  // prepare memory
  if (symmetrized_data_sequence->size() <
      static_cast<std::size_t>(output_length_)) {
    symmetrized_data_sequence->resize(output_length_);
  }

  switch (input_format_) {
    case kStandard: {
      switch (output_format_) {
        case kStandard: {
          std::copy(data_sequence.begin(), data_sequence.end(),
                    symmetrized_data_sequence->begin());
          break;
        }
        case kSymmetricForApplyingFourierTransform: {
          std::copy(data_sequence.begin(), data_sequence.end(),
                    symmetrized_data_sequence->begin());
          std::reverse_copy(data_sequence.begin() + 1, data_sequence.end() - 1,
                            symmetrized_data_sequence->begin() + input_length_);
          break;
        }
        case kSymmetricForFormingImpulseResponse: {
          std::reverse_copy(data_sequence.begin(), data_sequence.end(),
                            symmetrized_data_sequence->begin());
          std::copy(data_sequence.begin() + 1, data_sequence.end(),
                    symmetrized_data_sequence->begin() + input_length_);
          (*symmetrized_data_sequence)[0] *= 0.5;
          (*symmetrized_data_sequence)[fft_length_] *= 0.5;
          break;
        }
        case kSymmetricForPlottingImpulseResponse: {
          std::reverse_copy(data_sequence.begin(), data_sequence.end(),
                            symmetrized_data_sequence->begin());
          std::copy(data_sequence.begin() + 1, data_sequence.end(),
                    symmetrized_data_sequence->begin() + input_length_);
          break;
        }
        default: { return false; }
      }
      break;
    }

    case kSymmetricForApplyingFourierTransform: {
      switch (output_format_) {
        case kStandard: {
          std::copy(data_sequence.begin(),
                    data_sequence.begin() + fft_length_ / 2 + 1,
                    symmetrized_data_sequence->begin());
          break;
        }
        case kSymmetricForApplyingFourierTransform: {
          std::copy(data_sequence.begin(), data_sequence.end(),
                    symmetrized_data_sequence->begin());
          break;
        }
        case kSymmetricForFormingImpulseResponse: {
          std::copy(data_sequence.begin() + fft_length_ / 2,
                    data_sequence.end(), symmetrized_data_sequence->begin());
          std::copy(data_sequence.begin(),
                    data_sequence.begin() + fft_length_ / 2 + 1,
                    symmetrized_data_sequence->begin() + fft_length_ / 2);
          (*symmetrized_data_sequence)[0] *= 0.5;
          (*symmetrized_data_sequence)[fft_length_] *= 0.5;
          break;
        }
        case kSymmetricForPlottingImpulseResponse: {
          std::copy(data_sequence.begin() + fft_length_ / 2,
                    data_sequence.end(), symmetrized_data_sequence->begin());
          std::copy(data_sequence.begin(),
                    data_sequence.begin() + fft_length_ / 2 + 1,
                    symmetrized_data_sequence->begin() + fft_length_ / 2);
          break;
        }
        default: { return false; }
      }
      break;
    }

    case kSymmetricForFormingImpulseResponse: {
      switch (output_format_) {
        case kStandard: {
          std::copy(data_sequence.begin() + fft_length_ / 2,
                    data_sequence.end(), symmetrized_data_sequence->begin());
          (*symmetrized_data_sequence)[fft_length_ / 2] *= 2.0;
          break;
        }
        case kSymmetricForApplyingFourierTransform: {
          std::copy(data_sequence.begin() + fft_length_ / 2,
                    data_sequence.end(), symmetrized_data_sequence->begin());
          std::copy(data_sequence.begin() + 1,
                    data_sequence.begin() + fft_length_ / 2,
                    symmetrized_data_sequence->begin() + fft_length_ / 2 + 1);
          (*symmetrized_data_sequence)[fft_length_ / 2] *= 2.0;
          break;
        }
        case kSymmetricForFormingImpulseResponse: {
          std::copy(data_sequence.begin(), data_sequence.end(),
                    symmetrized_data_sequence->begin());
          break;
        }
        case kSymmetricForPlottingImpulseResponse: {
          std::copy(data_sequence.begin(), data_sequence.end(),
                    symmetrized_data_sequence->begin());
          (*symmetrized_data_sequence)[0] *= 2.0;
          (*symmetrized_data_sequence)[fft_length_] *= 2.0;
          break;
        }
        default: { return false; }
      }
      break;
    }

    case kSymmetricForPlottingImpulseResponse: {
      switch (output_format_) {
        case kStandard: {
          std::copy(data_sequence.begin() + fft_length_ / 2,
                    data_sequence.end(), symmetrized_data_sequence->begin());
          break;
        }
        case kSymmetricForApplyingFourierTransform: {
          std::copy(data_sequence.begin() + fft_length_ / 2,
                    data_sequence.end(), symmetrized_data_sequence->begin());
          std::copy(data_sequence.begin() + 1,
                    data_sequence.begin() + fft_length_ / 2,
                    symmetrized_data_sequence->begin() + fft_length_ / 2 + 1);
          break;
        }
        case kSymmetricForFormingImpulseResponse: {
          std::copy(data_sequence.begin(), data_sequence.end(),
                    symmetrized_data_sequence->begin());
          (*symmetrized_data_sequence)[0] *= 0.5;
          (*symmetrized_data_sequence)[fft_length_] *= 0.5;
          break;
        }
        case kSymmetricForPlottingImpulseResponse: {
          std::copy(data_sequence.begin(), data_sequence.end(),
                    symmetrized_data_sequence->begin());
          break;
        }
        default: { return false; }
      }
      break;
    }

    default: { return false; }
  }

  return true;
}

}  // namespace sptk
