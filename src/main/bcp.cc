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

#include <algorithm>  // std::max
#include <cstdint>    // int8_t, int16_t, int32_t, int64_t, etc.
#include <cstring>    // std::strncmp
#include <fstream>    // std::ifstream
#include <iomanip>    // std::setprecision, std::setw
#include <iostream>   // std::cerr, std::cin, std::cout, std::endl, etc.
#include <limits>     // std::numeric_limits
#include <sstream>    // std::ostringstream
#include <stdexcept>  // std::invalid_argument
#include <string>     // std::stold, std::string
#include <vector>     // std::vector

#include "Getopt/getoptwin.h"
#include "SPTK/utils/int24_t.h"
#include "SPTK/utils/sptk_utils.h"
#include "SPTK/utils/uint24_t.h"

namespace {

const int kDefaultInputStartNumber(0);
const int kDefaultInputBlockLength(512);
const int kDefaultOutputStartNumber(0);
const double kDefaultPadValue(0.0);
const char* kDefaultDataType("d");

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " bcp - block copy" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       bcp [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -s s  : start number (input)      (   int)[" << std::setw(5) << std::right << kDefaultInputStartNumber  << "][ 0 <= s <= e ]" << std::endl;  // NOLINT
  *stream << "       -e e  : end number (input)        (   int)[" << std::setw(5) << std::right << "l-1"                     << "][ s <= e <  l ]" << std::endl;  // NOLINT
  *stream << "       -l l  : block length (input)      (   int)[" << std::setw(5) << std::right << kDefaultInputBlockLength  << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : block order (input)       (   int)[" << std::setw(5) << std::right << "l-1"                     << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -S S  : start number (output)     (   int)[" << std::setw(5) << std::right << kDefaultOutputStartNumber << "][ 0 <= S <  L ]" << std::endl;  // NOLINT
  *stream << "       -L L  : block length (output)     (   int)[" << std::setw(5) << std::right << "N/A"                     << "][ 1 <= L <=   ]" << std::endl;  // NOLINT
  *stream << "       -M M  : block order (output)      (   int)[" << std::setw(5) << std::right << "N/A"                     << "][ 0 <= M <=   ]" << std::endl;  // NOLINT
  *stream << "       -f f  : pad value for empty slots (double)[" << std::setw(5) << std::right << kDefaultPadValue          << "][   <= f <=   ]" << std::endl;  // NOLINT
  *stream << "       +type : data type                         [" << std::setw(5) << std::right << kDefaultDataType          << "]" << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("c", stream); sptk::PrintDataType("C", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("s", stream); sptk::PrintDataType("S", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("h", stream); sptk::PrintDataType("H", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("i", stream); sptk::PrintDataType("I", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("l", stream); sptk::PrintDataType("L", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("f", stream); sptk::PrintDataType("d", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("e", stream); sptk::PrintDataType("a", stream); *stream << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence                     (  type)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       copied data sequence              (  type)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

class BlockCopyInterface {
 public:
  virtual ~BlockCopyInterface() {
  }

  virtual bool Run(std::istream* input_stream) const = 0;
};

template <typename T>
class BlockCopy : public BlockCopyInterface {
 public:
  BlockCopy(int input_start_number, int input_end_number,
            int input_block_length, int output_start_number,
            int output_block_length, T pad_value, bool is_ascii = false)
      : input_start_number_(input_start_number),
        input_end_number_(input_end_number),
        input_block_length_(input_block_length),
        output_start_number_(output_start_number),
        output_block_length_(output_block_length),
        pad_value_(pad_value),
        is_ascii_(is_ascii) {
  }

  ~BlockCopy() {
  }

  virtual bool Run(std::istream* input_stream) const {
    const int copy_length(input_end_number_ - input_start_number_ + 1);
    const int left_pad_length(output_start_number_);
    const int right_pad_length(output_block_length_ - output_start_number_ -
                               copy_length);

    std::vector<T> pad_data(std::max(left_pad_length, right_pad_length),
                            pad_value_);
    std::vector<T> input_data(input_block_length_);

    if (is_ascii_) {
      T* pads(&(pad_data[0]));
      T* inputs(&(input_data[0]));
      bool halt(false);
      while (!halt) {
        // Read data.
        for (int i(0); i < input_block_length_; ++i) {
          std::string word;
          *input_stream >> word;
          if (word.empty()) {
            halt = true;
            break;
          }
          if (input_start_number_ <= i && i <= input_end_number_) {
            try {
              inputs[i] = std::stold(word);
            } catch (std::invalid_argument&) {
              return false;
            }
          }
        }
        if (halt) break;

        // Write data.
        for (int i(0); i < left_pad_length; ++i) {
          std::cout << pads[i] << " ";
        }
        for (int i(input_start_number_); i <= input_end_number_; ++i) {
          std::cout << std::setprecision(
                           std::numeric_limits<long double>::digits10 + 1)
                    << inputs[i] << " ";
        }
        for (int i(0); i < right_pad_length; ++i) {
          std::cout << pads[i] << " ";
        }
        std::cout << std::endl;
      }
    } else {
      while (sptk::ReadStream(false, 0, 0, input_block_length_, &input_data,
                              input_stream, NULL)) {
        if (0 < left_pad_length &&
            !sptk::WriteStream(0, left_pad_length, pad_data, &std::cout,
                               NULL)) {
          return false;
        }
        if (!sptk::WriteStream(input_start_number_, copy_length, input_data,
                               &std::cout, NULL)) {
          return false;
        }
        if (0 < right_pad_length &&
            !sptk::WriteStream(0, right_pad_length, pad_data, &std::cout,
                               NULL)) {
          return false;
        }
      }
    }

    return true;
  }

 private:
  const int input_start_number_;
  const int input_end_number_;
  const int input_block_length_;
  const int output_start_number_;
  const int output_block_length_;
  const T pad_value_;
  const bool is_ascii_;

  DISALLOW_COPY_AND_ASSIGN(BlockCopy<T>);
};

class BlockCopyWrapper {
 public:
  BlockCopyWrapper(const std::string& data_type, int input_start_number,
                   int input_end_number, int input_block_length,
                   int output_start_number, int output_block_length,
                   double pad_value)
      : block_copy_(NULL) {
    if ("c" == data_type) {
      block_copy_ = new BlockCopy<int8_t>(
          input_start_number, input_end_number, input_block_length,
          output_start_number, output_block_length, pad_value);
    } else if ("s" == data_type) {
      block_copy_ = new BlockCopy<int16_t>(
          input_start_number, input_end_number, input_block_length,
          output_start_number, output_block_length, pad_value);
    } else if ("h" == data_type) {
      block_copy_ = new BlockCopy<sptk::int24_t>(
          input_start_number, input_end_number, input_block_length,
          output_start_number, output_block_length, sptk::int24_t(pad_value));
    } else if ("i" == data_type) {
      block_copy_ = new BlockCopy<int32_t>(
          input_start_number, input_end_number, input_block_length,
          output_start_number, output_block_length, pad_value);
    } else if ("l" == data_type) {
      block_copy_ = new BlockCopy<int64_t>(
          input_start_number, input_end_number, input_block_length,
          output_start_number, output_block_length, pad_value);
    } else if ("C" == data_type) {
      block_copy_ = new BlockCopy<uint8_t>(
          input_start_number, input_end_number, input_block_length,
          output_start_number, output_block_length, pad_value);
    } else if ("S" == data_type) {
      block_copy_ = new BlockCopy<uint16_t>(
          input_start_number, input_end_number, input_block_length,
          output_start_number, output_block_length, pad_value);
    } else if ("H" == data_type) {
      block_copy_ = new BlockCopy<sptk::uint24_t>(
          input_start_number, input_end_number, input_block_length,
          output_start_number, output_block_length, sptk::uint24_t(pad_value));
    } else if ("I" == data_type) {
      block_copy_ = new BlockCopy<uint32_t>(
          input_start_number, input_end_number, input_block_length,
          output_start_number, output_block_length, pad_value);
    } else if ("L" == data_type) {
      block_copy_ = new BlockCopy<uint64_t>(
          input_start_number, input_end_number, input_block_length,
          output_start_number, output_block_length, pad_value);
    } else if ("f" == data_type) {
      block_copy_ = new BlockCopy<float>(
          input_start_number, input_end_number, input_block_length,
          output_start_number, output_block_length, pad_value);
    } else if ("d" == data_type) {
      block_copy_ = new BlockCopy<double>(
          input_start_number, input_end_number, input_block_length,
          output_start_number, output_block_length, pad_value);
    } else if ("e" == data_type) {
      block_copy_ = new BlockCopy<long double>(
          input_start_number, input_end_number, input_block_length,
          output_start_number, output_block_length, pad_value);
    } else if ("a" == data_type) {
      block_copy_ = new BlockCopy<long double>(
          input_start_number, input_end_number, input_block_length,
          output_start_number, output_block_length, pad_value, true);
    }
  }

  ~BlockCopyWrapper() {
    delete block_copy_;
  }

  bool IsValid() const {
    return NULL != block_copy_;
  }

  bool Run(std::istream* input_stream) const {
    return IsValid() && block_copy_->Run(input_stream);
  }

 private:
  BlockCopyInterface* block_copy_;

  DISALLOW_COPY_AND_ASSIGN(BlockCopyWrapper);
};

}  // namespace

/**
 * @a bcp [ @e option ] [ @e infile ]
 *
 * - @b -s @e int
 *   - start number (input) @f$(0 \le s \le e)@f$
 * - @b -e @e int
 *   - end number (input) @f$(s \le e < l)@f$
 * - @b -l @e int
 *   - block length (input) @f$(1 \le l)@f$
 * - @b -m @e int
 *   - block order (input) @f$(0 \le n)@f$
 * - @b -S @e int
 *   - start number (output) @f$(0 \le S < L)@f$
 * - @b -L @e int
 *   - block length (output) @f$(1 \le L)@f$
 * - @b -M @e int
 *   - block order (output) @f$(0 \le N)@f$
 * - @b -f @e double
 *   - pad value @f$(f)@f$
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
 *     \arg @c a ascii
 * - @b infile @e str
 *   - input data sequence
 * - @b stdout
 *   - copied data sequence
 *
 * This command copies data blocks in a frame-by-frame manner.
 * The below figure shows the overview of the command.
 *
 * @image html bcp_1.png
 *
 * The following example extracts multiples of three from a ramp sequence.
 *
 * @code{.sh}
 *   ramp -s 1 -l 9 | bcp +d -s 2 -l 3 | x2x +da
 *   # 3, 6, 9
 *   ramp -s 1 -l 9 | bcp +d -s 2 -l 3 -L 2 | x2x +da
 *   # 3, 0, 6, 0, 9, 0
 *   ramp -s 1 -l 9 | bcp +d -s 2 -l 3 -L 2 -S 1 | x2x +da
 *   # 0, 3, 0, 6, 0, 9
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int input_start_number(kDefaultInputStartNumber);
  int input_end_number(kDefaultInputBlockLength - 1);
  int input_block_length(kDefaultInputBlockLength);
  int output_start_number(kDefaultOutputStartNumber);
  int output_block_length(kDefaultInputBlockLength);
  double pad_value(kDefaultPadValue);
  std::string data_type(kDefaultDataType);
  bool is_input_end_number_specified(false);
  bool is_output_block_length_specified(false);

  for (;;) {
    const int option_char(
        getopt_long(argc, argv, "s:e:l:m:S:L:M:f:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 's': {
        if (!sptk::ConvertStringToInteger(optarg, &input_start_number) ||
            input_start_number < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -s option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("bcp", error_message);
          return 1;
        }
        break;
      }
      case 'e': {
        if (!sptk::ConvertStringToInteger(optarg, &input_end_number) ||
            input_end_number < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -e option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("bcp", error_message);
          return 1;
        }
        is_input_end_number_specified = true;
        break;
      }
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &input_block_length) ||
            input_block_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("bcp", error_message);
          return 1;
        }
        break;
      }
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &input_block_length) ||
            input_block_length < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("bcp", error_message);
          return 1;
        }
        ++input_block_length;
        break;
      }
      case 'S': {
        if (!sptk::ConvertStringToInteger(optarg, &output_start_number) ||
            output_start_number < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -S option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("bcp", error_message);
          return 1;
        }
        break;
      }
      case 'L': {
        if (!sptk::ConvertStringToInteger(optarg, &output_block_length) ||
            output_block_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -L option must be a positive integer";
          sptk::PrintErrorMessage("bcp", error_message);
          return 1;
        }
        is_output_block_length_specified = true;
        break;
      }
      case 'M': {
        if (!sptk::ConvertStringToInteger(optarg, &output_block_length) ||
            output_block_length < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -M option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("bcp", error_message);
          return 1;
        }
        ++output_block_length;
        is_output_block_length_specified = true;
        break;
      }
      case 'f': {
        if (!sptk::ConvertStringToDouble(optarg, &pad_value)) {
          std::ostringstream error_message;
          error_message << "The argument for the -f option must be numeric";
          sptk::PrintErrorMessage("bcp", error_message);
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

  if (!is_input_end_number_specified) {
    input_end_number = input_block_length - 1;
  } else if (input_block_length <= input_end_number) {
    std::ostringstream error_message;
    error_message << "End number " << input_end_number
                  << " must be less than block length " << input_block_length;
    sptk::PrintErrorMessage("bcp", error_message);
    return 1;
  } else if (input_end_number < input_start_number) {
    std::ostringstream error_message;
    error_message << "End number " << input_end_number
                  << " must be equal to or greater than start number "
                  << input_start_number;
    sptk::PrintErrorMessage("bcp", error_message);
    return 1;
  }

  if (input_block_length <= input_start_number) {
    std::ostringstream error_message;
    error_message << "Start number " << input_start_number
                  << " must be less than block length " << input_block_length;
    sptk::PrintErrorMessage("bcp", error_message);
    return 1;
  }

  const int copy_length(input_end_number - input_start_number + 1);
  if (!is_output_block_length_specified) {
    output_block_length = output_start_number + copy_length;
  } else if (output_block_length < output_start_number + copy_length) {
    std::ostringstream error_message;
    error_message << "Output block length is too short";
    sptk::PrintErrorMessage("bcp", error_message);
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
      sptk::PrintErrorMessage("bcp", error_message);
      return 1;
    }
  }

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("bcp", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  BlockCopyWrapper block_copy(data_type, input_start_number, input_end_number,
                              input_block_length, output_start_number,
                              output_block_length, pad_value);

  if (!block_copy.IsValid()) {
    std::ostringstream error_message;
    error_message << "Unexpected argument for the +type option";
    sptk::PrintErrorMessage("bcp", error_message);
    return 1;
  }

  if (!block_copy.Run(&input_stream)) {
    std::ostringstream error_message;
    error_message << "Failed to copy";
    sptk::PrintErrorMessage("bcp", error_message);
    return 1;
  }

  return 0;
}
