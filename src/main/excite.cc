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

#include <exception>  // std::exception
#include <fstream>    // std::ifstream
#include <iomanip>    // std::setw
#include <iostream>   // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>    // std::ostringstream

#include "GETOPT/ya_getopt.h"
#include "SPTK/generation/excitation_generation.h"
#include "SPTK/generation/m_sequence_generation.h"
#include "SPTK/generation/normal_distributed_random_value_generation.h"
#include "SPTK/input/input_source_from_stream.h"
#include "SPTK/input/input_source_interpolation_with_magic_number.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum NoiseType { kZero = 0, kGaussian, kMSequence, kNumNoiseTypes };

const int kDefaultFramePeriod(100);
const int kDefaultInterpolationPeriod(1);
const NoiseType kDefaultNoiseType(NoiseType::kMSequence);
const int kDefaultSeed(1);
const double kMagicNumberForUnvoicedFrame(0.0);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " excite - generate excitation" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       excite [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -p p  : frame period               (   int)[" << std::setw(5) << std::right << kDefaultFramePeriod         << "][ 1 <= p <=     ]" << std::endl;  // NOLINT
  *stream << "       -i i  : interpolation period       (   int)[" << std::setw(5) << std::right << kDefaultInterpolationPeriod << "][ 0 <= i <= p/2 ]" << std::endl;  // NOLINT
  *stream << "       -n n  : noise type                 (   int)[" << std::setw(5) << std::right << kDefaultNoiseType           << "][ 0 <= n <= 2   ]" << std::endl;  // NOLINT
  *stream << "                 0 (none)" << std::endl;
  *stream << "                 1 (Gaussian)" << std::endl;
  *stream << "                 2 (M-sequence)" << std::endl;
  *stream << "       -s s  : seed for random generation (   int)[" << std::setw(5) << std::right << kDefaultSeed                << "][   <= s <=     ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       pitch period                       (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       excitation                         (double)" << std::endl;
  *stream << "  notice:" << std::endl;
  *stream << "       if i = 0, don't interpolate pitch" << std::endl;
  *stream << "       s is valid only if n = 1" << std::endl;
  *stream << "       magic number for unvoiced frame is " << kMagicNumberForUnvoicedFrame << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a excite [ @e option ] [ @e infile ]
 *
 * - @b -p @e int
 *   - frame_period @f$(1 \le P)@f$
 * - @b -i @e int
 *   - interpolation period @f$(0 \le I \le P/2)@f$
 * - @b -n @e int
 *   - noise type
 *     \arg @c 0 none
 *     \arg @c 1 Gaussian
 *     \arg @c 2 M-sequence
 * - @b -s @e int
 *   - seed for random number generation
 * - @b infile @e str
 *   - double-type pitch period
 * - @b stdout
 *   - double-type excitation
 *
 * This command generates an excitation sequence from the pitch period in
 * @c infile (or standard input), and sends the result to standard output.
 * When the pitch period is nonzero, the excitation is to be a pulse train.
 * When the pitch period is zero (i.e., unvoiced), the excitation is to be
 * a Gaussian or M-sequence noise.
 *
 * In the example below, the excitation is generated from the @c data.p and
 * passed through an LPC synthesis filter. The speech signal is written to
 * @c data.syn file.
 *
 * @code{.sh}
 *   excite < data.p | poledf data.lpc > data.syn
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int frame_period(kDefaultFramePeriod);
  int interpolation_period(kDefaultInterpolationPeriod);
  NoiseType noise_type(kDefaultNoiseType);
  int seed(kDefaultSeed);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "p:i:n:s:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'p': {
        if (!sptk::ConvertStringToInteger(optarg, &frame_period) ||
            frame_period <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -p option must be a positive integer";
          sptk::PrintErrorMessage("excite", error_message);
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
          sptk::PrintErrorMessage("excite", error_message);
          return 1;
        }
        break;
      }
      case 'n': {
        const int min(0);
        const int max(static_cast<int>(kNumNoiseTypes) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -n option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("excite", error_message);
          return 1;
        }
        noise_type = static_cast<NoiseType>(tmp);
        break;
      }
      case 's': {
        if (!sptk::ConvertStringToInteger(optarg, &seed)) {
          std::ostringstream error_message;
          error_message << "The argument for the -s option must be an integer";
          sptk::PrintErrorMessage("excite", error_message);
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

  if (frame_period / 2 < interpolation_period) {
    std::ostringstream error_message;
    error_message << "Interpolation period must be equal to or less than half "
                  << "frame period";
    sptk::PrintErrorMessage("excite", error_message);
    return 1;
  }

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("excite", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  if (!sptk::SetBinaryMode()) {
    std::ostringstream error_message;
    error_message << "Cannot set translation mode";
    sptk::PrintErrorMessage("excite", error_message);
    return 1;
  }

  std::ifstream ifs;
  if (NULL != input_file) {
    ifs.open(input_file, std::ios::in | std::ios::binary);
    if (ifs.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << input_file;
      sptk::PrintErrorMessage("excite", error_message);
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
    sptk::PrintErrorMessage("excite", error_message);
    return 1;
  }

  sptk::RandomGenerationInterface* random_generation(NULL);
  try {
    switch (noise_type) {
      case kZero: {
        break;
      }
      case kGaussian: {
        random_generation =
            new sptk::NormalDistributedRandomValueGeneration(seed);
        break;
      }
      case kMSequence: {
        random_generation = new sptk::MSequenceGeneration();
        break;
      }
      default: {
        return 1;
      }
    }
    sptk::ExcitationGeneration excitation_generation(
        &input_source_interpolation_with_magic_number, random_generation);

    if (!excitation_generation.IsValid()) {
      std::ostringstream error_message;
      error_message << "Failed to initialize ExcitationGeneration";
      sptk::PrintErrorMessage("excite", error_message);
      delete random_generation;
      return 1;
    }

    double excitation;
    while (excitation_generation.Get(&excitation, NULL, NULL, NULL)) {
      if (!sptk::WriteStream(excitation, &std::cout)) {
        std::ostringstream error_message;
        error_message << "Failed to write excitation";
        sptk::PrintErrorMessage("excite", error_message);
        delete random_generation;
        return 1;
      }
    }
  } catch (std::exception&) {
    std::ostringstream error_message;
    error_message << "Unknown exception";
    sptk::PrintErrorMessage("excite", error_message);
    delete random_generation;
    return 1;
  }
  delete random_generation;

  return 0;
}
