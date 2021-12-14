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

#include <cctype>    // std::isprint
#include <fstream>   // std::ifstream
#include <iomanip>   // std::setfill, std::setw
#include <ios>       // std::dec, std::hex, std::oct
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream
#include <string>    // std::string

#include "Getopt/getoptwin.h"
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
  *stream << "       data sequence              (   any)[stdin]" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       dumped data sequence       (string)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a fd [ @e option ] [ @e infile ]
 *
 * - @b -s @e int
 *   - start index length @f$(0 \le S)@f$
 * - @b -c @e int
 *   - number of columns @f$(1 \le N)@f$
 * - @b -o @e address format
 *   - data type
 *     \arg @c 0 none
 *     \arg @c 1 hexadecimal
 *     \arg @c 2 decimal
 *     \arg @c 3 octal
 * - @b infile @e str
 *   - data sequence
 * - @b stdout
 *   - dumped data sequence
 *
 * This command converts data from @c infile (or standard input) to a human
 * readable multi-column form, and sends the result to standard output.
 *
 * The below example displays the speech data in @c sample.wav with the
 * corresponding addresses.
 *
 * @code{.sh}
 * fd -o 1 sample.wav
 * # 000000  52 49 46 46 5e 9d 05 00 57 41 56 45 66 6d 74 20 |RIFF^...WAVEfmt |
 * # 000010  10 00 00 00 01 00 01 00 22 56 00 00 44 ac 00 00 |........"V..D...|
 * # 000020  02 00 10 00 64 61 74 61 3a 9d 05 00 05 00 03 00 |....data:.......|
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
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

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("fd", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

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
    // Output address.
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
        default: {
          break;
        }
      }
    }

    // Stack human-readable characters.
    if (std::isprint(data)) {
      stored_characters << data;
    } else {
      stored_characters << ".";
    }

    // Output data.
    std::cout << std::setfill('0') << std::setw(2) << std::hex
              << static_cast<int>(data) << " ";

    // Output new line.
    if (num_column - 1 == (index - start_index) % num_column) {
      std::cout << "|" << stored_characters.str() << "|\n";
      stored_characters.str("");  // Clear.
    }
  }

  // Flush.
  const std::string residual_characters(stored_characters.str());
  if (!residual_characters.empty()) {
    const std::string space(3 * (num_column - residual_characters.size()), ' ');
    std::cout << space << "|" << residual_characters << "|" << std::endl;
  }

  return 0;
}
