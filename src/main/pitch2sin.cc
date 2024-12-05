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

#include "GETOPT/ya_getopt.h"
#include "SPTK/generation/periodic_waveform_generation.h"
#include "SPTK/input/input_source_from_stream.h"
#include "SPTK/input/input_source_interpolation_with_magic_number.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum OutputFormats { kSine = 0, kCosine, kSawtooth, kNumOutputFormats };

const int kDefaultFramePeriod(100);
const int kDefaultInterpolationPeriod(1);
const OutputFormats kDefaultOutputFormat(kSine);
const bool kDefaultStrictFlag(false);
const double kDefaultUnvoicedValue(0.0);
const double kMagicNumberForUnvoicedFrame(0.0);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " pitch2sin - convert pitch to sinusoidal sequence" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       pitch2sin [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -p p  : frame period         (   int)[" << std::setw(5) << std::right << kDefaultFramePeriod         << "][ 1 <= p <=     ]" << std::endl;  // NOLINT
  *stream << "       -i i  : interpolation period (   int)[" << std::setw(5) << std::right << kDefaultInterpolationPeriod << "][ 0 <= i <= p/2 ]" << std::endl;  // NOLINT
  *stream << "       -o o  : output format        (   int)[" << std::setw(5) << std::right << kDefaultOutputFormat        << "][ 0 <= o <= 2   ]" << std::endl;  // NOLINT
  *stream << "                 0 (sine)" << std::endl;
  *stream << "                 1 (cosine)" << std::endl;
  *stream << "                 2 (sawtooth)" << std::endl;
  *stream << "       -u u  : value on unvoiced    (double)[" << std::setw(5) << std::right << kDefaultUnvoicedValue       << "][   <= u <=     ]" << std::endl;  // NOLINT
  *stream << "               region" << std::endl;
  *stream << "       -s    : strictly drop signal (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultStrictFlag) << "]" << std::endl;  // NOLINT
  *stream << "               in unvoiced region" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       pitch period                 (double)[stdin]" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       sinusoidal sequence          (double)" << std::endl;
  *stream << "  notice:" << std::endl;
  *stream << "       if i = 0, don't interpolate pitch" << std::endl;
  *stream << "       magic number for unvoiced frame is " << kMagicNumberForUnvoicedFrame << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a pitch2sin [ @e option ] [ @e infile ]
 *
 * - @b -p @e int
 *   - frame_period @f$(1 \le P)@f$
 * - @b -i @e int
 *   - interpolation period @f$(0 \le I \le P/2)@f$
 * - @b -o @e int
 *   - output format
 *     \arg @c 0 sine
 *     \arg @c 1 cosine
 *     \arg @c 2 sawtooth
 * - @b -u @e double
 *   - value on unvoiced region
 * - @b -s
 *   - strictly drop sinusoidal in unvoiced region
 * - @b infile @e str
 *   - double-type pitch period
 * - @b stdout
 *   - double-type sinusoidal sequence
 *
 * The below is a simple example to generate sinusoidal from @c data.d.
 *
 * @code{.sh}
 *   pitch -s 16 -p 80 -o 0 < data.d | pitch2sin -p 80 > data.sin
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int frame_period(kDefaultFramePeriod);
  int interpolation_period(kDefaultInterpolationPeriod);
  OutputFormats output_format(kDefaultOutputFormat);
  double unvoiced_value(kDefaultUnvoicedValue);
  bool strict(kDefaultStrictFlag);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "p:i:o:u:sh", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'p': {
        if (!sptk::ConvertStringToInteger(optarg, &frame_period) ||
            frame_period <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -p option must be a positive integer";
          sptk::PrintErrorMessage("pitch2sin", error_message);
          return 1;
        }
        break;
      }
      case 'i': {
        if (!sptk::ConvertStringToInteger(optarg, &interpolation_period) ||
            interpolation_period < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -i option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("pitch2sin", error_message);
          return 1;
        }
        break;
      }
      case 'o': {
        const int min(0);
        const int max(static_cast<int>(kNumOutputFormats) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -o option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("pitch2sin", error_message);
          return 1;
        }
        output_format = static_cast<OutputFormats>(tmp);
        break;
      }
      case 'u': {
        if (!sptk::ConvertStringToDouble(optarg, &unvoiced_value)) {
          std::ostringstream error_message;
          error_message << "The argument for the -u option must be a number";
          sptk::PrintErrorMessage("pitch2sin", error_message);
          return 1;
        }
        break;
      }
      case 's': {
        strict = true;
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

  if (frame_period / 2 < interpolation_period) {
    std::ostringstream error_message;
    error_message << "Interpolation period must be equal to or less than half "
                  << "frame period";
    sptk::PrintErrorMessage("pitch2sin", error_message);
    return 1;
  }

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("pitch2sin", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  if (!sptk::SetBinaryMode()) {
    std::ostringstream error_message;
    error_message << "Cannot set translation mode";
    sptk::PrintErrorMessage("pitch2sin", error_message);
    return 1;
  }

  std::ifstream ifs;
  if (NULL != input_file) {
    ifs.open(input_file, std::ios::in | std::ios::binary);
    if (ifs.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << input_file;
      sptk::PrintErrorMessage("pitch2sin", error_message);
      return 1;
    }
  }
  std::istream& input_stream(ifs.is_open() ? ifs : std::cin);

  sptk::InputSourceFromStream input_source_from_stream(false, 1, &input_stream);
  sptk::InputSourceInterpolationWithMagicNumber
      input_source_interpolation_with_magic_number(
          frame_period, interpolation_period, false,
          kMagicNumberForUnvoicedFrame, &input_source_from_stream);
  if (!input_source_interpolation_with_magic_number.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize InputSourceFromStream";
    sptk::PrintErrorMessage("pitch2sin", error_message);
    return 1;
  }

  sptk::PeriodicWaveformGeneration waveform_generation(
      unvoiced_value, strict, &input_source_interpolation_with_magic_number);
  if (!waveform_generation.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize PeriodicWaveformGeneration";
    sptk::PrintErrorMessage("pitch2sin", error_message);
    return 1;
  }

  double signal;
  while (waveform_generation.Get(kSine == output_format ? &signal : NULL,
                                 kCosine == output_format ? &signal : NULL,
                                 kSawtooth == output_format ? &signal : NULL,
                                 NULL)) {
    if (!sptk::WriteStream(signal, &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write a periodic waveform";
      sptk::PrintErrorMessage("pitch2sin", error_message);
      return 1;
    }
  }

  return 0;
}
