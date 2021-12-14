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

#include "Getopt/getoptwin.h"
#include "SPTK/compression/mu_law_compression.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const double kDefaultAbsMaxValue(32768.0);
const double kDefaultCompressionFactor(255);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " ulaw - u-law pulse code modulation" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       ulaw [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -v v  : absolute maximum of input (double)[" << std::setw(5) << std::right << kDefaultAbsMaxValue       << "][ 0.0 <  v <=   ]" << std::endl;  // NOLINT
  *stream << "       -u u  : compression factor        (double)[" << std::setw(5) << std::right << kDefaultCompressionFactor << "][ 0.0 <  u <=   ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       input sequence                    (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       compressed sequence               (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a ulaw [ @e option ] [ @e infile ]
 *
 * - @b -v @e int
 *   - absolute maximum value of input @f$(0 < V)@f$
 * - @b -u @e double
 *   - compression factor @f$(0 < \mu)@f$
 * - @b infile @e str
 *   - double-type input data sequence
 * - @b stdout
 *   - double-type compressed data sequence
 *
 * In the below example, 16-bit data read from @c data.short is compressed to
 * 8-bit ulaw format.
 *
 * @code{.sh}
 *   x2x +sd data.short | ulaw | quantize > data.ulaw
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
          sptk::PrintErrorMessage("ulaw", error_message);
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
          sptk::PrintErrorMessage("ulaw", error_message);
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
    sptk::PrintErrorMessage("ulaw", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("ulaw", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::MuLawCompression mu_law_compression(abs_max_value, compression_factor);
  if (!mu_law_compression.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize MuLawCompression";
    sptk::PrintErrorMessage("ulaw", error_message);
    return 1;
  }

  double data;

  while (sptk::ReadStream(&data, &input_stream)) {
    if (!mu_law_compression.Run(&data)) {
      std::ostringstream error_message;
      error_message << "Failed to compress";
      sptk::PrintErrorMessage("ulaw", error_message);
      return 1;
    }

    if (!sptk::WriteStream(data, &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write compressed data";
      sptk::PrintErrorMessage("ulaw", error_message);
      return 1;
    }
  }

  return 0;
}
