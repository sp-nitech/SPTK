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

#include <algorithm>  // std::reverse
#include <climits>    // INT_MAX
#include <cstdint>    // int16_t, int32_t, int64_t, etc.
#include <cstring>    // std::strncmp
#include <fstream>    // std::ifstream
#include <iomanip>    // std::setw
#include <iostream>   // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>    // std::ostringstream
#include <string>     // std::string

#include "Getopt/getoptwin.h"
#include "SPTK/utils/int24_t.h"
#include "SPTK/utils/sptk_utils.h"
#include "SPTK/utils/uint24_t.h"

namespace {

const int kDefaultStartAddress(0);
const int kDefaultStartOffset(0);
const int kDefaultEndAddress(INT_MAX);
const int kDefaultEndOffset(INT_MAX);
const char* kDefaultDataType("s");

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " swab - swap bytes" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       swab [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -S S  : start address      (   int)[" << std::setw(5) << std::right << kDefaultStartAddress << "][ 0 <= S <= E ]" << std::endl;  // NOLINT
  *stream << "       -s s  : start offset       (   int)[" << std::setw(5) << std::right << kDefaultStartOffset  << "][ 0 <= s <= e ]" << std::endl;  // NOLINT
  *stream << "       -E E  : end address        (   int)[" << std::setw(5) << std::right << "EOF"                << "][ S <= E <=   ]" << std::endl;  // NOLINT
  *stream << "       -e e  : end offset         (   int)[" << std::setw(5) << std::right << "EOF"                << "][ s <= e <=   ]" << std::endl;  // NOLINT
  *stream << "       +type : data type                  [" << std::setw(5) << std::right << kDefaultDataType     << "]" << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("s", stream); sptk::PrintDataType("S", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("h", stream); sptk::PrintDataType("H", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("i", stream); sptk::PrintDataType("I", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("l", stream); sptk::PrintDataType("L", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("f", stream); sptk::PrintDataType("d", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("e", stream);                                   *stream << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence              (  type)[" << std::setw(5) << std::right << "stdin" << "]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       swapped data sequence      (  type)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

class ByteSwapInterface {
 public:
  virtual ~ByteSwapInterface() {
  }

  virtual bool Run(std::istream* input_stream) const = 0;
};

template <typename T>
class ByteSwap : public ByteSwapInterface {
 public:
  ByteSwap(int start_address, int start_offset, int end_address, int end_offset)
      : start_address_(start_address),
        start_offset_(start_offset),
        end_address_(end_address),
        end_offset_(end_offset) {
  }

  ~ByteSwap() {
  }

  virtual bool Run(std::istream* input_stream) const {
    // Skip data.
    const int data_size(static_cast<int>(sizeof(T)));
    const int skip_size(start_address_ + data_size * start_offset_);
    if (&std::cin == input_stream) {
      unsigned char tmp;
      int i(0);
      while (i < skip_size && sptk::ReadStream(&tmp, input_stream)) {
        ++i;
      }
    } else {
      input_stream->seekg(skip_size);
    }
    input_stream->peek();
    if (!input_stream->good()) {
      return false;
    }

    // Swap data.
    T data;
    for (int address(skip_size), offset(start_offset_);
         ((address <= end_address_) && (offset <= end_offset_) &&
          sptk::ReadStream(&data, input_stream));
         address += data_size, ++offset) {
      unsigned char* p(reinterpret_cast<unsigned char*>(&data));
      std::reverse(p, p + data_size);
      if (!sptk::WriteStream(data, &std::cout)) {
        return false;
      }
    }

    return true;
  }

 private:
  const int start_address_;
  const int start_offset_;
  const int end_address_;
  const int end_offset_;

  DISALLOW_COPY_AND_ASSIGN(ByteSwap<T>);
};

class ByteSwapWrapper {
 public:
  ByteSwapWrapper(const std::string& data_type, int start_address,
                  int start_offset, int end_address, int end_offset)
      : byte_swap_(NULL) {
    if ("s" == data_type) {
      byte_swap_ = new ByteSwap<int16_t>(start_address, start_offset,
                                         end_address, end_offset);
    } else if ("h" == data_type) {
      byte_swap_ = new ByteSwap<sptk::int24_t>(start_address, start_offset,
                                               end_address, end_offset);
    } else if ("i" == data_type) {
      byte_swap_ = new ByteSwap<int32_t>(start_address, start_offset,
                                         end_address, end_offset);
    } else if ("l" == data_type) {
      byte_swap_ = new ByteSwap<int64_t>(start_address, start_offset,
                                         end_address, end_offset);
    } else if ("S" == data_type) {
      byte_swap_ = new ByteSwap<uint16_t>(start_address, start_offset,
                                          end_address, end_offset);
    } else if ("H" == data_type) {
      byte_swap_ = new ByteSwap<sptk::uint24_t>(start_address, start_offset,
                                                end_address, end_offset);
    } else if ("I" == data_type) {
      byte_swap_ = new ByteSwap<uint32_t>(start_address, start_offset,
                                          end_address, end_offset);
    } else if ("L" == data_type) {
      byte_swap_ = new ByteSwap<uint64_t>(start_address, start_offset,
                                          end_address, end_offset);
    } else if ("f" == data_type) {
      byte_swap_ = new ByteSwap<float>(start_address, start_offset, end_address,
                                       end_offset);
    } else if ("d" == data_type) {
      byte_swap_ = new ByteSwap<double>(start_address, start_offset,
                                        end_address, end_offset);
    } else if ("e" == data_type) {
      byte_swap_ = new ByteSwap<long double>(start_address, start_offset,
                                             end_address, end_offset);
    }
  }

  ~ByteSwapWrapper() {
    delete byte_swap_;
  }

  bool IsValid() const {
    return NULL != byte_swap_;
  }

  bool Run(std::istream* input_stream) const {
    return IsValid() && byte_swap_->Run(input_stream);
  }

 private:
  ByteSwapInterface* byte_swap_;

  DISALLOW_COPY_AND_ASSIGN(ByteSwapWrapper);
};

}  // namespace

/**
 * @a swab [ @e option ] [ @e infile ]
 *
 * - @b -S @e int
 *   - start address @f$(0 \le S)@f$
 * - @b -s @e int
 *   - start offset @f$(0 \le s)@f$
 * - @b -E @e int
 *   - end address @f$(S \le E)@f$
 * - @b -e @e int
 *   - end offset @f$(s \le e)@f$
 * - @b +type @e char
 *   - data type
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
 * - @b infile @e str
 *   - data sequence
 * - @b stdout
 *   - swapped data sequence
 *
 * This command changes the byte order (from big-endian to little-endian or vice
 * versa) of the input data read from @c infile.
 *
 * The beginning and the end of the swapping can be controlled. The beginning is
 * @f$S + Ts@f$ and the end is @f$\min(E, Te)@f$, where @f$T@f$ is the size of
 * the data type decided by @c +type option.
 *
 * The following example skips the header of 12 bytes long and changes the byte
 * order.
 *
 * @code{.sh}
 *   swab +f -S 12 htk.mfcc | dmp +f
 * @endcode
 *
 * Swapped data can be reverted as below.
 *
 * @code{.sh}
 *   ramp -l 4 | x2x +fs | swab +s | swab +s | x2x +sa
 *   # 0, 1, 2, 3
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int start_address(kDefaultStartAddress);
  int start_offset(kDefaultStartOffset);
  int end_address(kDefaultEndAddress);
  int end_offset(kDefaultEndOffset);
  std::string data_type(kDefaultDataType);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "S:s:E:e:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'S': {
        if (!sptk::ConvertStringToInteger(optarg, &start_address) ||
            start_address < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -S option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("swab", error_message);
          return 1;
        }
        break;
      }
      case 's': {
        if (!sptk::ConvertStringToInteger(optarg, &start_offset) ||
            start_offset < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -s option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("swab", error_message);
          return 1;
        }
        break;
      }
      case 'E': {
        if (!sptk::ConvertStringToInteger(optarg, &end_address) ||
            end_address < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -E option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("swab", error_message);
          return 1;
        }
        break;
      }
      case 'e': {
        if (!sptk::ConvertStringToInteger(optarg, &end_offset) ||
            end_offset < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -e option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("swab", error_message);
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

  if (end_address < start_address) {
    std::ostringstream error_message;
    error_message << "End address must be equal to or greater than "
                  << "start address";
    sptk::PrintErrorMessage("swab", error_message);
    return 1;
  }

  if (end_offset < start_offset) {
    std::ostringstream error_message;
    error_message << "End offset number must be equal to or greater than "
                  << "start offset number";
    sptk::PrintErrorMessage("swab", error_message);
    return 1;
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
      sptk::PrintErrorMessage("swab", error_message);
      return 1;
    }
  }

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("swab", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  ByteSwapWrapper swap_byte(data_type, start_address, start_offset, end_address,
                            end_offset);

  if (!swap_byte.IsValid()) {
    std::ostringstream error_message;
    error_message << "Unexpected argument for the +type option";
    sptk::PrintErrorMessage("swab", error_message);
    return 1;
  }

  if (!swap_byte.Run(&input_stream)) {
    std::ostringstream error_message;
    error_message << "Failed to swap bytes";
    sptk::PrintErrorMessage("swab", error_message);
    return 1;
  }

  return 0;
}
