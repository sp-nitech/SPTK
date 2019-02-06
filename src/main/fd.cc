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

#include <getopt.h>  // getopt_long
#include <cctype>    // std::isprint
#include <fstream>   // std::ifstream
#include <iomanip>   // std::setfill, std::setw
#include <ios>       // std::dec, std::hex, std::oct
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream
#include <string>    // std::string

#include "SPTK/utils/sptk_utils.h"

namespace {

enum AddressFormats {
  kNone = 0,
  kHexadecimal,
  kDecimal,
  kOctal,
  kNumAddressFormats
};

const int kDefaultStartIndex(0);
const int kDefaultNumColumn(16);
const AddressFormats kDefaultAddressFormat(kNone);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " fd - file dump" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       fd [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -s s  : start index        (   int)[" << std::setw(5) << std::right << kDefaultStartIndex    << "][ 0 <= s <=   ]" << std::endl;  // NOLINT
  *stream << "       -c c  : number of columns  (   int)[" << std::setw(5) << std::right << kDefaultNumColumn     << "][ 1 <= c <=   ]" << std::endl;  // NOLINT
  *stream << "       -o o  : address format     (   int)[" << std::setw(5) << std::right << kDefaultAddressFormat << "][ 0 <= o <= 3 ]" << std::endl;  // NOLINT
  *stream << "                 0 (none)" << std::endl;
  *stream << "                 1 (hexadecimal)" << std::endl;
  *stream << "                 2 (decimal)" << std::endl;
  *stream << "                 3 (octal)" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence                      [stdin]" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       dumped data sequence" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

int main(int argc, char* argv[]) {
  int start_index(kDefaultStartIndex);
  int num_column(kDefaultNumColumn);
  AddressFormats address_format(kDefaultAddressFormat);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "s:c:o:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 's': {
        if (!sptk::ConvertStringToInteger(optarg, &start_index) ||
            start_index < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -s option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("fd", error_message);
          return 1;
        }
        break;
      }
      case 'c': {
        if (!sptk::ConvertStringToInteger(optarg, &num_column) ||
            num_column <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -c option must be a positive integer";
          sptk::PrintErrorMessage("fd", error_message);
          return 1;
        }
        break;
      }
      case 'o': {
        const int min(0);
        const int max(static_cast<int>(kNumAddressFormats) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -o option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("fd", error_message);
          return 1;
        }
        address_format = static_cast<AddressFormats>(tmp);
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
    sptk::PrintErrorMessage("fd", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  // open stream
  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("fd", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  uint8_t data;
  std::ostringstream stored_characters;

  for (int index(start_index); sptk::ReadStream(&data, &input_stream);
       ++index) {
    // output address
    if (0 == (index - start_index) % num_column) {
      switch (address_format) {
        case kNone: {
          // nothing to do
          break;
        }
        case kHexadecimal: {
          std::cout << std::setfill('0') << std::setw(6) << std::hex << index
                    << "  ";
          break;
        }
        case kDecimal: {
          std::cout << std::setfill('0') << std::setw(6) << std::dec << index
                    << "  ";
          break;
        }
        case kOctal: {
          std::cout << std::setfill('0') << std::setw(6) << std::oct << index
                    << "  ";
          break;
        }
        default: { break; }
      }
    }

    // stack human-readable characters
    if (std::isprint(data)) {
      stored_characters << data;
    } else {
      stored_characters << ".";
    }

    // output data
    std::cout << std::setfill('0') << std::setw(2) << std::hex
              << static_cast<int>(data) << " ";

    // output new line
    if (num_column - 1 == (index - start_index) % num_column) {
      std::cout << "|" << stored_characters.str() << "|" << std::endl;
      stored_characters.str("");  // clear
    }
  }

  // flush
  const std::string residual_characters(stored_characters.str());
  if (!residual_characters.empty()) {
    const std::string space(3 * (num_column - residual_characters.size()), ' ');
    std::cout << space << "|" << residual_characters << "|" << std::endl;
  }

  return 0;
}
