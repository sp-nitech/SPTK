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
//                1996-2018  Nagoya Institute of Technology          //
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

#include <getopt.h>   // getopt_long
#include <exception>  // std::exception
#include <fstream>    // std::ifstream
#include <iomanip>    // std::setw
#include <iostream>   // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>    // std::ostringstream

#include "SPTK/generator/excitation_generation.h"
#include "SPTK/generator/m_sequence_generation.h"
#include "SPTK/generator/normal_distributed_random_value_generation.h"
#include "SPTK/input/input_source_from_stream.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultFramePeriod(100);
const int kDefaultInterpolationPeriod(1);
const bool kDefaultFlagToUseNormalDistributedRandomValue(false);
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
  *stream << "       -p p  : frame period                       (   int)[" << std::setw(5) << std::right << kDefaultFramePeriod         << "][ 0 <  p <=     ]" << std::endl;  // NOLINT
  *stream << "       -i i  : interpolation period               (   int)[" << std::setw(5) << std::right << kDefaultInterpolationPeriod << "][ 0 <= i <= p/2 ]" << std::endl;  // NOLINT
  *stream << "       -n    : use gauss noise for unvoiced frame (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultFlagToUseNormalDistributedRandomValue) << "]" << std::endl;  // NOLINT
  *stream << "               default is M-sequence" << std::endl;
  *stream << "       -s s  : seed for random generation         (   int)[" << std::setw(5) << std::right << kDefaultSeed                << "][   <= s <=     ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       pitch period                               (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       excitation                                 (double)" << std::endl;  // NOLINT
  *stream << "  notice:" << std::endl;
  *stream << "       if i = 0, don't interpolate pitch" << std::endl;
  *stream << "       magic number for unvoiced frame is " << kMagicNumberForUnvoicedFrame << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

int main(int argc, char* argv[]) {
  int frame_period(kDefaultFramePeriod);
  int interpolation_period(kDefaultInterpolationPeriod);
  bool use_normal_distributed_random_value(
      kDefaultFlagToUseNormalDistributedRandomValue);
  int seed(kDefaultSeed);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "p:i:ns:h", NULL, NULL));
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
        use_normal_distributed_random_value = true;
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

  // Get input file name.
  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("excite", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  // Open input stream.
  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("excite", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  // Prepare input source interpolation.
  sptk::InputSourceFromStream input_source_from_stream(false, 1, &input_stream);
  sptk::InputSourceInterpolationWithMagicNumber
      input_source_interpolation_with_magic_number(
          frame_period, interpolation_period, false,
          kMagicNumberForUnvoicedFrame, &input_source_from_stream);
  if (!input_source_interpolation_with_magic_number.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to set the condition for input";
    sptk::PrintErrorMessage("excite", error_message);
    return 1;
  }

  // Run excitation generation.
  sptk::RandomGenerationInterface* random_generation(NULL);
  try {
    if (use_normal_distributed_random_value) {
      random_generation =
          new sptk::NormalDistributedRandomValueGeneration(seed);
    } else {
      random_generation = new sptk::MSequenceGeneration();
    }
    sptk::ExcitationGeneration excitation_generation(
        &input_source_interpolation_with_magic_number, random_generation);

    if (!excitation_generation.IsValid()) {
      std::ostringstream error_message;
      error_message << "Failed to set condition for excitation generation";
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
