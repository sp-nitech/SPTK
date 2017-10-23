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
//                1996-2017  Nagoya Institute of Technology          //
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
#include <algorithm>
#include <climits>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "SPTK/utils/int24_t.h"
#include "SPTK/utils/sptk_utils.h"
#include "SPTK/utils/uint24_t.h"

namespace {

const int kDefaultStartAddress(0);
const int kDefaultStartOffsetNumber(0);
const int kDefaultEndAddress(INT_MAX);
const int kDefaultEndOffsetNumber(INT_MAX);
const char* kDefaultDataType("s");

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " swab - swap bytes" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       swab [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -S S  : start address       (   int)[" << std::setw(10) << std::right << kDefaultStartAddress      << "][ 0 <= S <= E ]" << std::endl;  // NOLINT
  *stream << "       -s s  : start offset number (   int)[" << std::setw(10) << std::right << kDefaultStartOffsetNumber << "][ 0 <= s <= e ]" << std::endl;  // NOLINT
  *stream << "       -E E  : end address         (   int)[" << std::setw(10) << std::right << kDefaultEndAddress        << "][ S <= E <=   ]" << std::endl;  // NOLINT
  *stream << "       -e e  : end offset number   (   int)[" << std::setw(10) << std::right << kDefaultEndOffsetNumber   << "][ s <= e <=   ]" << std::endl;  // NOLINT
  *stream << "       +type : data type                   [" << std::setw(10) << std::right << kDefaultDataType          << "]" << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("s", stream); sptk::PrintDataType("S", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("h", stream); sptk::PrintDataType("H", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("i", stream); sptk::PrintDataType("I", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("l", stream); sptk::PrintDataType("L", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("f", stream); sptk::PrintDataType("d", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("e", stream);                                   *stream << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence                       [" << std::setw(10) << std::right << "stdin" << "]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       swapped data sequence" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

class ByteSwapInterface {
 public:
  virtual ~ByteSwapInterface() {
  }

  virtual bool Run() = 0;
};

template <typename T>
class ByteSwap : public ByteSwapInterface {
 public:
  ByteSwap(int start_address, int start_offset_number, int end_address,
           int end_offset_number, std::istream* input_stream)
      : start_address_(start_address),
        start_offset_number_(start_offset_number),
        end_address_(end_address),
        end_offset_number_(end_offset_number),
        input_stream_(input_stream) {
  }

  ~ByteSwap() {
  }

  virtual bool Run() {
    // Skip data
    const int data_size(static_cast<int>(sizeof(T)));
    const int skip_size(start_address_ + data_size * start_offset_number_);
    if (&std::cin == input_stream_) {
      unsigned char tmp;
      for (int i(0); i < skip_size && sptk::ReadStream(&tmp, input_stream_);
           ++i) {
      }
    } else {
      input_stream_->seekg(skip_size);
    }
    input_stream_->peek();
    if (!input_stream_->good()) {
      return false;
    }

    // Swap data
    T data;
    for (int address(skip_size), offset(start_offset_number_);
         ((address <= end_address_) && (offset <= end_offset_number_) &&
          sptk::ReadStream(&data, input_stream_));
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
  const int start_offset_number_;
  const int end_address_;
  const int end_offset_number_;
  std::istream* input_stream_;

  DISALLOW_COPY_AND_ASSIGN(ByteSwap<T>);
};

class ByteSwapWrapper {
 public:
  ByteSwapWrapper(const std::string& data_type, int start_address,
                  int start_offset_number, int end_address,
                  int end_offset_number, std::istream* input_stream) {
    if ("s" == data_type) {
      byte_swap_ =
          new ByteSwap<int16_t>(start_address, start_offset_number, end_address,
                                end_offset_number, input_stream);
    } else if ("h" == data_type) {
      byte_swap_ = new ByteSwap<sptk::int24_t>(start_address,
                                               start_offset_number, end_address,
                                               end_offset_number, input_stream);
    } else if ("i" == data_type) {
      byte_swap_ =
          new ByteSwap<int32_t>(start_address, start_offset_number, end_address,
                                end_offset_number, input_stream);
    } else if ("l" == data_type) {
      byte_swap_ =
          new ByteSwap<int64_t>(start_address, start_offset_number, end_address,
                                end_offset_number, input_stream);
    } else if ("S" == data_type) {
      byte_swap_ =
          new ByteSwap<uint16_t>(start_address, start_offset_number,
                                 end_address, end_offset_number, input_stream);
    } else if ("H" == data_type) {
      byte_swap_ = new ByteSwap<sptk::uint24_t>(
          start_address, start_offset_number, end_address, end_offset_number,
          input_stream);
    } else if ("I" == data_type) {
      byte_swap_ =
          new ByteSwap<uint32_t>(start_address, start_offset_number,
                                 end_address, end_offset_number, input_stream);
    } else if ("L" == data_type) {
      byte_swap_ =
          new ByteSwap<uint64_t>(start_address, start_offset_number,
                                 end_address, end_offset_number, input_stream);
    } else if ("f" == data_type) {
      byte_swap_ =
          new ByteSwap<float>(start_address, start_offset_number, end_address,
                              end_offset_number, input_stream);
    } else if ("d" == data_type) {
      byte_swap_ =
          new ByteSwap<double>(start_address, start_offset_number, end_address,
                               end_offset_number, input_stream);
    } else if ("e" == data_type) {
      byte_swap_ = new ByteSwap<long double>(start_address, start_offset_number,
                                             end_address, end_offset_number,
                                             input_stream);
    } else {
      byte_swap_ = NULL;
    }
  }

  ~ByteSwapWrapper() {
    delete byte_swap_;
  }

  bool IsValid() const {
    return NULL != byte_swap_;
  }

  bool Run() const {
    return IsValid() && byte_swap_->Run();
  }

 private:
  ByteSwapInterface* byte_swap_;

  DISALLOW_COPY_AND_ASSIGN(ByteSwapWrapper);
};

}  // namespace

int main(int argc, char* argv[]) {
  int start_address(kDefaultStartAddress);
  int start_offset_number(kDefaultStartOffsetNumber);
  int end_address(kDefaultEndAddress);
  int end_offset_number(kDefaultEndOffsetNumber);
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
        if (!sptk::ConvertStringToInteger(optarg, &start_offset_number) ||
            start_offset_number < 0) {
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
        if (!sptk::ConvertStringToInteger(optarg, &end_offset_number) ||
            end_offset_number < 0) {
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

  if (end_offset_number < start_offset_number) {
    std::ostringstream error_message;
    error_message << "End offset number must be equal to or greater than "
                  << "start offset number";
    sptk::PrintErrorMessage("swab", error_message);
    return 1;
  }

  // get input file
  const char* input_file(NULL);
  for (int i(argc - optind); 1 <= i; --i) {
    const char* arg = argv[argc - i];
    if (0 == std::strncmp(arg, "+", 1)) {
      const std::string str(arg);
      data_type = str.substr(1, std::string::npos);
    } else {
      input_file = arg;
    }
  }

  // open stream
  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("swab", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  ByteSwapWrapper swap_byte(data_type, start_address, start_offset_number,
                            end_address, end_offset_number, &input_stream);

  if (!swap_byte.IsValid()) {
    std::ostringstream error_message;
    error_message << "Unexpected argument for the +type option";
    sptk::PrintErrorMessage("swab", error_message);
    return 1;
  }

  if (!swap_byte.Run()) {
    std::ostringstream error_message;
    error_message << "Failed to swap bytes";
    sptk::PrintErrorMessage("swab", error_message);
    return 1;
  }

  return 0;
}
