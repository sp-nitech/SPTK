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

#include <cstring>   // std::strncmp
#include <fstream>   // std::ifstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream
#include <string>    // std::string

#include "Getopt/getoptwin.h"
#include "SPTK/utils/int24_t.h"
#include "SPTK/utils/sptk_utils.h"
#include "SPTK/utils/uint24_t.h"

namespace {

const int kBufferSize(128);
const int kMagicNumberForEndOfFile(-1);
const char* kDefaultDataType("d");

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " dmp - binary file dump" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       dmp [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : block length       (   int)[" << std::setw(5) << std::right << "EOS"            << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : block order        (   int)[" << std::setw(5) << std::right << "EOS"            << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -f f  : print format       (string)[" << std::setw(5) << std::right << "N/A"            << "]" << std::endl;  // NOLINT
  *stream << "       +type : data type                  [" << std::setw(5) << std::right << kDefaultDataType << "]" << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("c", stream); sptk::PrintDataType("C", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("s", stream); sptk::PrintDataType("S", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("h", stream); sptk::PrintDataType("H", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("i", stream); sptk::PrintDataType("I", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("l", stream); sptk::PrintDataType("L", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("f", stream); sptk::PrintDataType("d", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("e", stream);                                   *stream << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence              (  type)[stdin]" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       dumped data sequence       (string)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

class DataDumpInterface {
 public:
  virtual ~DataDumpInterface() {
  }

  virtual bool Run(std::istream* input_stream) const = 0;
};

template <typename T>
class DataDump : public DataDumpInterface {
 public:
  DataDump(const std::string& print_format, int minimum_index,
           int maximum_index)
      : print_format_(print_format),
        minimum_index_(minimum_index),
        maximum_index_(maximum_index) {
  }

  ~DataDump() {
  }

  virtual bool Run(std::istream* input_stream) const {
    char buffer[kBufferSize];
    T data;
    for (int index(minimum_index_); sptk::ReadStream(&data, input_stream);
         ++index) {
      // Format data.
      if (!sptk::SnPrintf(data, print_format_, sizeof(buffer), buffer)) {
        return false;
      }

      // Dump data. Note that std::endl makes the execution speed slow.
      std::cout << index << '\t' << buffer << '\n';

      // Initialize index.
      if (maximum_index_ != kMagicNumberForEndOfFile &&
          maximum_index_ == index) {
        index = minimum_index_ - 1;
      }
    }
    return true;
  }

 private:
  const std::string print_format_;
  const int minimum_index_;
  const int maximum_index_;

  DISALLOW_COPY_AND_ASSIGN(DataDump<T>);
};

class DataDumpWrapper {
 public:
  DataDumpWrapper(const std::string& data_type,
                  const std::string& given_print_format, int minimum_index,
                  int maximum_index)
      : data_dump_(NULL) {
    std::string print_format(given_print_format);
    if ("c" == data_type) {
      if (print_format.empty()) print_format = "%d";
      data_dump_ =
          new DataDump<int8_t>(print_format, minimum_index, maximum_index);
    } else if ("s" == data_type) {
      if (print_format.empty()) print_format = "%d";
      data_dump_ =
          new DataDump<int16_t>(print_format, minimum_index, maximum_index);
    } else if ("h" == data_type) {
      if (print_format.empty()) print_format = "%d";
      data_dump_ = new DataDump<sptk::int24_t>(print_format, minimum_index,
                                               maximum_index);
    } else if ("i" == data_type) {
      if (print_format.empty()) print_format = "%d";
      data_dump_ =
          new DataDump<int32_t>(print_format, minimum_index, maximum_index);
    } else if ("l" == data_type) {
      if (print_format.empty()) print_format = "%lld";
      data_dump_ =
          new DataDump<int64_t>(print_format, minimum_index, maximum_index);
    } else if ("C" == data_type) {
      if (print_format.empty()) print_format = "%u";
      data_dump_ =
          new DataDump<uint8_t>(print_format, minimum_index, maximum_index);
    } else if ("S" == data_type) {
      if (print_format.empty()) print_format = "%u";
      data_dump_ =
          new DataDump<uint16_t>(print_format, minimum_index, maximum_index);
    } else if ("H" == data_type) {
      if (print_format.empty()) print_format = "%u";
      data_dump_ = new DataDump<sptk::uint24_t>(print_format, minimum_index,
                                                maximum_index);
    } else if ("I" == data_type) {
      if (print_format.empty()) print_format = "%u";
      data_dump_ =
          new DataDump<uint32_t>(print_format, minimum_index, maximum_index);
    } else if ("L" == data_type) {
      if (print_format.empty()) print_format = "%llu";
      data_dump_ =
          new DataDump<uint64_t>(print_format, minimum_index, maximum_index);
    } else if ("f" == data_type) {
      if (print_format.empty()) print_format = "%g";
      data_dump_ =
          new DataDump<float>(print_format, minimum_index, maximum_index);
    } else if ("d" == data_type) {
      if (print_format.empty()) print_format = "%g";
      data_dump_ =
          new DataDump<double>(print_format, minimum_index, maximum_index);
    } else if ("e" == data_type) {
      if (print_format.empty()) print_format = "%Lg";
      data_dump_ =
          new DataDump<long double>(print_format, minimum_index, maximum_index);
    }
  }

  ~DataDumpWrapper() {
    delete data_dump_;
  }

  bool IsValid() const {
    return NULL != data_dump_;
  }

  bool Run(std::istream* input_stream) const {
    return IsValid() && data_dump_->Run(input_stream);
  }

 private:
  DataDumpInterface* data_dump_;

  DISALLOW_COPY_AND_ASSIGN(DataDumpWrapper);
};

}  // namespace

/**
 * @a dmp [ @e option ] [ @e infile ]
 *
 * - @b -l @e int
 *   - block length @f$(1 \le L)@f$
 * - @b -m @e int
 *   - block order @f$(0 \le L-1)@f$
 * - @b +type @e char
 *   - data type
 *     \arg @c c char (1byte)
 *     \arg @c C unsigned char (1byte)
 *     \arg @c s short (2byte)
 *     \arg @c S unsigned short (2byte)
 *     \arg @c h int (3byte)
 *     \arg @c H unsigned int (3byte)
 *     \arg @c i int (4byte)
 *     \arg @c I unsigned int (4byte)
 *     \arg @c l long (8byte)
 *     \arg @c L unsigned long (8byte)
 *     \arg @c f float (4byte)
 *     \arg @c d double (8byte)
 *     \arg @c e long double (16byte)
 * - @b -f @e str
 *   - print format
 * - @b infile @e str
 *   - data sequence
 * - @b stdout
 *   - dumped data sequence
 *
 * This command converts data from @c infile (or standard input) to a human
 * readable form (one sample per line, with line numbers), and sends the result
 * to standard output.
 *
 * @code{.sh}
 *   # 1, 2, 3, 4
 *   ramp -s 1 -l 4 | dmp
 *   # 0       1
 *   # 1       2
 *   # 2       3
 *   # 3       4
 * @endcode
 *
 * @code{.sh}
 *   ramp -s 1 -l 4 | dmp -l 2
 *   # 1       1
 *   # 2       2
 *   # 1       3
 *   # 2       4
 * @endcode
 *
 * @code{.sh}
 *   ramp -s 1 -l 4 | dmp -m 2 -f %.1f
 *   # 0       1.0
 *   # 1       2.0
 *   # 0       3.0
 *   # 1       4.0
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int minimum_index(0);
  int maximum_index(kMagicNumberForEndOfFile);
  std::string print_format("");
  std::string data_type(kDefaultDataType);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:f:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &maximum_index) ||
            maximum_index <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("dmp", error_message);
          return 1;
        }
        minimum_index = 1;
        break;
      }
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &maximum_index) ||
            maximum_index < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("dmp", error_message);
          return 1;
        }
        minimum_index = 0;
        break;
      }
      case 'f': {
        print_format = optarg;
        if ("%" != print_format.substr(0, 1)) {
          std::ostringstream error_message;
          error_message << "The argument for the -f option must be begin with "
                        << "%";
          sptk::PrintErrorMessage("dmp", error_message);
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

  const char* input_file(NULL);
  for (int i(argc - optind); 1 <= i; --i) {
    const char* arg(argv[argc - i]);
    if (0 == std::strncmp(arg, "+", 1)) {
      const std::string str(arg);
      data_type = str.substr(1, std::string::npos);
    } else if (NULL == input_file) {
      input_file = arg;
    } else {
      std::ostringstream error_message;
      error_message << "Too many input files";
      sptk::PrintErrorMessage("dmp", error_message);
      return 1;
    }
  }

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("dmp", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  DataDumpWrapper data_dump(data_type, print_format, minimum_index,
                            maximum_index);

  if (!data_dump.IsValid()) {
    std::ostringstream error_message;
    error_message << "Unexpected argument for the +type option";
    sptk::PrintErrorMessage("dmp", error_message);
    return 1;
  }

  if (!data_dump.Run(&input_stream)) {
    std::ostringstream error_message;
    error_message << "Failed to dump";
    sptk::PrintErrorMessage("dmp", error_message);
    return 1;
  }

  return 0;
}
