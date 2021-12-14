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

#include "SPTK/utils/data_symmetrizing.h"

#include <algorithm>  // std::copy, std::reverse_copy
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
        kSymmetricForPreservingFrequencyResponse: {
      return fft_length + 1;
    }
    case sptk::DataSymmetrizing::InputOutputFormats::
        kSymmetricForPlottingFrequencyResponse: {
      return fft_length + 1;
    }
    default: {
      return 0;
    }
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
  if (fft_length_ < 2 || !IsEven(fft_length_) || 0 == input_length_ ||
      0 == output_length_) {
    is_valid_ = false;
    return;
  }
}

bool DataSymmetrizing::Run(
    const std::vector<double>& data_sequence,
    std::vector<double>* symmetrized_data_sequence) const {
  // Check inputs.
  if (!is_valid_ ||
      data_sequence.size() != static_cast<std::size_t>(input_length_) ||
      NULL == symmetrized_data_sequence) {
    return false;
  }

  // Prepare memories.
  if (symmetrized_data_sequence->size() !=
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
        case kSymmetricForPreservingFrequencyResponse: {
          std::reverse_copy(data_sequence.begin(), data_sequence.end(),
                            symmetrized_data_sequence->begin());
          std::copy(data_sequence.begin() + 1, data_sequence.end(),
                    symmetrized_data_sequence->begin() + input_length_);
          (*symmetrized_data_sequence)[0] *= 0.5;
          (*symmetrized_data_sequence)[fft_length_] *= 0.5;
          break;
        }
        case kSymmetricForPlottingFrequencyResponse: {
          std::reverse_copy(data_sequence.begin(), data_sequence.end(),
                            symmetrized_data_sequence->begin());
          std::copy(data_sequence.begin() + 1, data_sequence.end(),
                    symmetrized_data_sequence->begin() + input_length_);
          break;
        }
        default: {
          return false;
        }
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
        case kSymmetricForPreservingFrequencyResponse: {
          std::copy(data_sequence.begin() + fft_length_ / 2,
                    data_sequence.end(), symmetrized_data_sequence->begin());
          std::copy(data_sequence.begin(),
                    data_sequence.begin() + fft_length_ / 2 + 1,
                    symmetrized_data_sequence->begin() + fft_length_ / 2);
          (*symmetrized_data_sequence)[0] *= 0.5;
          (*symmetrized_data_sequence)[fft_length_] *= 0.5;
          break;
        }
        case kSymmetricForPlottingFrequencyResponse: {
          std::copy(data_sequence.begin() + fft_length_ / 2,
                    data_sequence.end(), symmetrized_data_sequence->begin());
          std::copy(data_sequence.begin(),
                    data_sequence.begin() + fft_length_ / 2 + 1,
                    symmetrized_data_sequence->begin() + fft_length_ / 2);
          break;
        }
        default: {
          return false;
        }
      }
      break;
    }

    case kSymmetricForPreservingFrequencyResponse: {
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
        case kSymmetricForPreservingFrequencyResponse: {
          std::copy(data_sequence.begin(), data_sequence.end(),
                    symmetrized_data_sequence->begin());
          break;
        }
        case kSymmetricForPlottingFrequencyResponse: {
          std::copy(data_sequence.begin(), data_sequence.end(),
                    symmetrized_data_sequence->begin());
          (*symmetrized_data_sequence)[0] *= 2.0;
          (*symmetrized_data_sequence)[fft_length_] *= 2.0;
          break;
        }
        default: {
          return false;
        }
      }
      break;
    }

    case kSymmetricForPlottingFrequencyResponse: {
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
        case kSymmetricForPreservingFrequencyResponse: {
          std::copy(data_sequence.begin(), data_sequence.end(),
                    symmetrized_data_sequence->begin());
          (*symmetrized_data_sequence)[0] *= 0.5;
          (*symmetrized_data_sequence)[fft_length_] *= 0.5;
          break;
        }
        case kSymmetricForPlottingFrequencyResponse: {
          std::copy(data_sequence.begin(), data_sequence.end(),
                    symmetrized_data_sequence->begin());
          break;
        }
        default: {
          return false;
        }
      }
      break;
    }

    default: {
      return false;
    }
  }

  return true;
}

}  // namespace sptk
