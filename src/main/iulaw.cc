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
//                1996-2020  Nagoya Institute of Technology          //
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

#include <getopt.h>  // getopt_long

#include <fstream>   // std::ifstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream

#include "SPTK/compression/mu_law_expansion.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const double kDefaultAbsMaxValue(32768.0);
const double kDefaultCompressionFactor(255);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " iulaw - inverse u-law pulse code modulation" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       iulaw [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -v v  : absolute maximum of input (double)[" << std::setw(5) << std::right << kDefaultAbsMaxValue       << "][ 0.0 <  v <=   ]" << std::endl;  // NOLINT
  *stream << "       -u u  : compression factor        (   int)[" << std::setw(5) << std::right << kDefaultCompressionFactor << "][ 0.0 <  u <=   ]" << std::endl;  // NOLINT
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

/**
 * @a iulaw [ @e option ] [ \e infile ]
 *
 * - @b -v @e int
 *   - absolute maximum value of input @f$(0 < V)@f$
 * - @b -u @e double
 *   - compression factor @f$(0 < \mu)@f$
 * - @b infile @e str
 *   - double-type compressed data sequence
 * - @b stdout
 *   - double-type output data sequence
 *
 * In the below example, 8-bit compressed and quantized data read from
 * @c data.ulaw is transformed into raw waveform.
 *
 * @code{.sh}
 *   dequantize data.ulaw | iulaw > data.raw
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  double abs_max_value(kDefaultAbsMaxValue);
  double compression_factor(kDefaultCompressionFactor);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "v:u:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'v': {
        if (!sptk::ConvertStringToDouble(optarg, &abs_max_value) ||
            abs_max_value <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -v option must be a positive number";
          sptk::PrintErrorMessage("iulaw", error_message);
          return 1;
        }
        break;
      }
      case 'u': {
        if (!sptk::ConvertStringToDouble(optarg, &compression_factor) ||
            compression_factor <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -u option must be a positive number";
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

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("iulaw", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("iulaw", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::MuLawExpansion mu_law_expansion(abs_max_value, compression_factor);
  if (!mu_law_expansion.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize MuLawExpansion";
    sptk::PrintErrorMessage("iulaw", error_message);
    return 1;
  }

  double data;

  while (sptk::ReadStream(&data, &input_stream)) {
    if (!mu_law_expansion.Run(&data)) {
      std::ostringstream error_message;
      error_message << "Failed to decompress";
      sptk::PrintErrorMessage("iulaw", error_message);
      return 1;
    }

    if (!sptk::WriteStream(data, &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write decompressed data";
      sptk::PrintErrorMessage("iulaw", error_message);
      return 1;
    }
  }

  return 0;
}
