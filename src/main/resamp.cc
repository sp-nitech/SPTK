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

#include <fstream>   // std::ifstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream
#include <vector>    // std::vector

#include "GETOPT/ya_getopt.h"
#include "SPTK/resampler/scalar_resampler.h"
#include "SPTK/resampler/vector_resampler.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kBufferLength(1024);

const int kDefaultVectorLength(1);
const double kDefaultInputSampleRate(16.0);
const double kDefaultOutputSampleRate(48.0);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " resamp - sampling rate conversion" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       resamp [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : length of vector           (   int)[" << std::setw(5) << std::right << kDefaultVectorLength       << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : order of vector            (   int)[" << std::setw(5) << std::right << "l-1"                      << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -s s  : input sampling rate [kHz]  (double)[" << std::setw(5) << std::right << kDefaultInputSamplingRate  << "][ 0 <  s <=   ]" << std::endl;  // NOLINT
  *stream << "       -S S  : output sampling rate [kHz] (double)[" << std::setw(5) << std::right << kDefaultOutputSamplingRate << "][ 0 <  S <=   ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence                      (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       resampled data sequence            (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a resamp [ @e option ] [ @e infile ]
 *
 * - @b -l @e int
 *   - length of vector @f$(1 \le L)@f$
 * - @b -m @e int
 *   - order of vector @f$(0 \le M)@f$
 * - @b -s @e double
 *   - input sampling rate in kHz
 * - @b -S @e double
 *   - output sampling rate in kHz
 * - @b infile @e str
 *   - double-type data sequence
 * - @b stdout
 *   - double-type resampled data sequence
 *
 * The example command converts 16 kHz data to 44.1 kHz data.
 *
 * @code{.sh}
 *   resamp -s 16 -S 44.1 < data.raw > data_us.raw
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int vector_length(kDefaultVectorLength);
  double input_sampling_rate(kDefaultInputSampleRate);
  double output_sampling_rate(kDefaultOutputSampleRate);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:s:S:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &vector_length) ||
            vector_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("resamp", error_message);
          return 1;
        }
        break;
      }
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &vector_length) ||
            vector_length < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("resamp", error_message);
          return 1;
        }
        ++vector_length;
        break;
      }
      case 's': {
        if (!sptk::ConvertStringToDouble(optarg, &input_sampling_rate) ||
            input_sampling_rate <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -s option must be a positive number";
          sptk::PrintErrorMessage("resamp", error_message);
          return 1;
        }
        break;
      }
      case 'S': {
        if (!sptk::ConvertStringToDouble(optarg, &output_sampling_rate) ||
            output_sampling_rate <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -S option must be a positive number";
          sptk::PrintErrorMessage("resamp", error_message);
          return 1;
        }
        break;
      }
      case 'h': {
        PrintUsage(&std::cout);
        return 0;
      }
      default: {
        PrintUsage(&std::cerr);
        return 1;
      }
    }
  }

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("resamp", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  if (!sptk::SetBinaryMode()) {
    std::ostringstream error_message;
    error_message << "Cannot set translation mode";
    sptk::PrintErrorMessage("resamp", error_message);
    return 1;
  }

  std::ifstream ifs;
  if (NULL != input_file) {
    ifs.open(input_file, std::ios::in | std::ios::binary);
    if (ifs.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << input_file;
      sptk::PrintErrorMessage("resamp", error_message);
      return 1;
    }
  }
  std::istream& input_stream(ifs.is_open() ? ifs : std::cin);

  if (1 == vector_length) {
    sptk::ScalarResampler resampler(input_sampling_rate, output_sampling_rate);
    sptk::ScalarResampler::Buffer buffer;
    if (!resampler.IsValid()) {
      std::ostringstream error_message;
      error_message << "Failed to initialize ScalarResampler";
      sptk::PrintErrorMessage("resamp", error_message);
      return 1;
    }

    std::vector<double> inputs(kBufferLength);
    std::vector<double> outputs;
    int read_size;

    while (sptk::ReadStream(true, 0, 0, kBufferLength, &inputs[0], &input_stream,
                            &read_size)) {
      if (kBufferLength != read_size) {
        inputs.resize(read_size);
      }
      if (!resampler.Run(inputs, &outputs, &buffer)) {
        std::ostringstream error_message;
        error_message << "Failed to perform resampling";
        sptk::PrintErrorMessage("resamp", error_message);
        return 1;
      }
      if (!sptk::WriteStream(0, static_cast<int>(output.size()), outputs, &std::cout, NULL)) {
        std::ostringstream error_message;
        error_message << "Failed to write resampled data";
        sptk::PrintErrorMessage("resamp", error_message);
        return 1;
      }
    }
  } else {
    sptk::VectorResampler resampler(
        vector_length, input_sampling_rate, output_sampling_rate);
    sptk::VectorResampler::Buffer buffer;
    if (!resampler.IsValid()) {
      std::ostringstream error_message;
      error_message << "Failed to initialize VectorResampler";
      sptk::PrintErrorMessage("resamp", error_message);
      return 1;
    }

    std::vector<std::vector<double> > inputs(1, std::vector<double>(vector_length));
    std::vector<std::vector<double> > outputs;

    while (sptk::ReadStream(false, 0, 0, vector_length, &inputs[0], &input_stream,
                            NULL)) {
      if (!resampler.Run(inputs, &outputs, &buffer)) {
        std::ostringstream error_message;
        error_message << "Failed to perform resampling";
        sptk::PrintErrorMessage("resamp", error_message);
        return 1;
      }
      for (const std::vector<double>& output : outputs) {
        if (!sptk::WriteStream(0, vector_length, output, &std::cout, NULL)) {
          std::ostringstream error_message;
          error_message << "Failed to write resampled vector";
          sptk::PrintErrorMessage("resamp", error_message);
          return 1;
        }
      }
    }
  }

  return 0;
}
