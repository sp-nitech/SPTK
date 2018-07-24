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

#include <getopt.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "SPTK/quantizer/inverse_mu_law_compression.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const double kDefaultAbsoluteMaxValue(32768);
const int kDefaultCompressionFactor(255);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " iulaw - inverse u-law pulse code modulation" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       iulaw [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -v v  : absolute maximum of input (double)[" << std::setw(5) << std::right << kDefaultAbsoluteMaxValue  << "][ 0.0 <  v <=   ]" << std::endl;  // NOLINT
  *stream << "       -u u  : compression factor        (   int)[" << std::setw(5) << std::right << kDefaultCompressionFactor << "][   0 <  u <=   ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       input sequence                    (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       decompressed sequence             (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

int main(int argc, char* argv[]) {
  double absolute_max_value(kDefaultAbsoluteMaxValue);
  int compression_factor(kDefaultCompressionFactor);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "v:u:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'v': {
        if (!sptk::ConvertStringToDouble(optarg, &absolute_max_value) ||
            absolute_max_value <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -v option must be a positive number";
          sptk::PrintErrorMessage("iulaw", error_message);
          return 1;
        }
        break;
      }
      case 'u': {
        if (!sptk::ConvertStringToInteger(optarg, &compression_factor) ||
            compression_factor <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -u option must be a positive integer";
          sptk::PrintErrorMessage("iulaw", error_message);
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

  // get input file
  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("iulaw", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  // open stream
  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("iulaw", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  // prepare for u-law decompression
  sptk::InverseMuLawCompression inverse_mu_law_compression(absolute_max_value,
                                                           compression_factor);
  if (!inverse_mu_law_compression.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to set condition for u-Law decompression";
    sptk::PrintErrorMessage("iulaw", error_message);
    return 1;
  }

  double input;
  double output;

  while (sptk::ReadStream(&input, &input_stream)) {
    if (!inverse_mu_law_compression.Run(input, &output)) {
      std::ostringstream error_message;
      error_message << "Failed to decompress";
      sptk::PrintErrorMessage("iulaw", error_message);
      return 1;
    }

    if (!sptk::WriteStream(output, &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write a decompressed sequence";
      sptk::PrintErrorMessage("iulaw", error_message);
      return 1;
    }
  }

  return 0;
}
