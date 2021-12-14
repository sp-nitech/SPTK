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

#include <cstdint>   // int8_t, int16_t, int32_t, int64_t, etc.
#include <cstring>   // std::strncmp
#include <fstream>   // std::ifstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream
#include <string>    // std::string
#include <vector>    // std::vector

#include "Getopt/getoptwin.h"
#include "SPTK/utils/int24_t.h"
#include "SPTK/utils/sptk_utils.h"
#include "SPTK/utils/uint24_t.h"

namespace {

const int kMagicNumberForEndOfFile(-1);
const int kDefaultStartNumber(0);
const int kDefaultEndNumber(kMagicNumberForEndOfFile);
const int kDefaultBlockLength(1);
const char* kDefaultDataType("d");

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " bcut - binary file cut" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       bcut [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -s s  : start number       (   int)[" << std::setw(5) << std::right << kDefaultStartNumber << "][ 0 <= s <= e ]" << std::endl;  // NOLINT
  *stream << "       -e e  : end number         (   int)[" << std::setw(5) << std::right << "EOF"               << "][ s <= e <=   ]" << std::endl;  // NOLINT
  *stream << "       -l l  : block length       (   int)[" << std::setw(5) << std::right << kDefaultBlockLength << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : block order        (   int)[" << std::setw(5) << std::right << "l-1"               << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       +type : data type                  [" << std::setw(5) << std::right << kDefaultDataType    << "]" << std::endl;  // NOLINT
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
  *stream << "       cut data sequence          (  type)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

class BinaryCutInterface {
 public:
  virtual ~BinaryCutInterface() {
  }

  virtual bool Run(std::istream* input_stream) const = 0;
};

template <typename T>
class BinaryCut : public BinaryCutInterface {
 public:
  BinaryCut(int start_number, int end_number, int block_length)
      : start_number_(start_number),
        end_number_(end_number),
        block_length_(block_length) {
  }

  ~BinaryCut() {
  }

  virtual bool Run(std::istream* input_stream) const {
    std::vector<T> data(block_length_);

    // Skip data.
    for (int block_index(0); block_index < start_number_; ++block_index) {
      if (!sptk::ReadStream(false, 0, 0, block_length_, &data, input_stream,
                            NULL)) {
        return false;
      }
    }

    // Write data.
    for (int block_index(start_number_);
         ((kMagicNumberForEndOfFile == end_number_ ||
           block_index <= end_number_) &&
          sptk::ReadStream(false, 0, 0, block_length_, &data, input_stream,
                           NULL));
         ++block_index) {
      if (!sptk::WriteStream(0, block_length_, data, &std::cout, NULL)) {
        return false;
      }
    }

    return true;
  }

 private:
  const int start_number_;
  const int end_number_;
  const int block_length_;

  DISALLOW_COPY_AND_ASSIGN(BinaryCut<T>);
};

class BinaryCutWrapper {
 public:
  BinaryCutWrapper(const std::string& data_type, int start_number,
                   int end_number, int block_length)
      : binary_cut_(NULL) {
    if ("c" == data_type) {
      binary_cut_ =
          new BinaryCut<int8_t>(start_number, end_number, block_length);
    } else if ("s" == data_type) {
      binary_cut_ =
          new BinaryCut<int16_t>(start_number, end_number, block_length);
    } else if ("h" == data_type) {
      binary_cut_ =
          new BinaryCut<sptk::int24_t>(start_number, end_number, block_length);
    } else if ("i" == data_type) {
      binary_cut_ =
          new BinaryCut<int32_t>(start_number, end_number, block_length);
    } else if ("l" == data_type) {
      binary_cut_ =
          new BinaryCut<int64_t>(start_number, end_number, block_length);
    } else if ("C" == data_type) {
      binary_cut_ =
          new BinaryCut<uint8_t>(start_number, end_number, block_length);
    } else if ("S" == data_type) {
      binary_cut_ =
          new BinaryCut<uint16_t>(start_number, end_number, block_length);
    } else if ("H" == data_type) {
      binary_cut_ =
          new BinaryCut<sptk::uint24_t>(start_number, end_number, block_length);
    } else if ("I" == data_type) {
      binary_cut_ =
          new BinaryCut<uint32_t>(start_number, end_number, block_length);
    } else if ("L" == data_type) {
      binary_cut_ =
          new BinaryCut<uint64_t>(start_number, end_number, block_length);
    } else if ("f" == data_type) {
      binary_cut_ =
          new BinaryCut<float>(start_number, end_number, block_length);
    } else if ("d" == data_type) {
      binary_cut_ =
          new BinaryCut<double>(start_number, end_number, block_length);
    } else if ("e" == data_type) {
      binary_cut_ =
          new BinaryCut<long double>(start_number, end_number, block_length);
    }
  }

  ~BinaryCutWrapper() {
    delete binary_cut_;
  }

  bool IsValid() const {
    return NULL != binary_cut_;
  }

  bool Run(std::istream* input_stream) const {
    return IsValid() && binary_cut_->Run(input_stream);
  }

 private:
  BinaryCutInterface* binary_cut_;

  DISALLOW_COPY_AND_ASSIGN(BinaryCutWrapper);
};

}  // namespace

/**
 * @a bcut [ @e option ] [ @e infile ]
 *
 * - @b -s @e int
 *   - start number @f$(0 \le S)@f$
 * - @b -e @e int
 *   - end number @f$(S \le E)@f$
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
 * - @b infile @e str
 *   - data sequence
 * - @b stdout
 *   - cut data sequence
 *
 * The input of this command is
 * @f[
 *   \begin{array}{ccc}
 *     \underbrace{x(0), \; \ldots, \; x(L - 1)}_{x_0}, &
 *     \underbrace{x(L), \; \ldots, \; x(2L - 1)}_{x_1}, &
 *     \ldots,
 *   \end{array}
 * @f]
 * and the output is
 * @f[
 *   \begin{array}{cccc}
 *     x_S, & x_{S+1}, & \ldots, & x_E,
 *   \end{array}
 * @f]
 * where @f$L@f$ is the block length.
 *
 * @code{.sh}
 *   ramp -l 7 | bcut +d -s 3 -e 5 | x2x +da
 *   # 3, 4, 5
 * @endcode
 *
 * @code{.sh}
 *   ramp -l 7 | bcut +d -l 2 -s 1 -e 1 | x2x +da
 *   # 2, 3
 * @endcode
 */
int main(int argc, char* argv[]) {
  int start_number(kDefaultStartNumber);
  int end_number(kDefaultEndNumber);
  int block_length(kDefaultBlockLength);
  std::string data_type(kDefaultDataType);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "s:e:l:m:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 's': {
        if (!sptk::ConvertStringToInteger(optarg, &start_number) ||
            start_number < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -s option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("bcut", error_message);
          return 1;
        }
        break;
      }
      case 'e': {
        if (!sptk::ConvertStringToInteger(optarg, &end_number) ||
            end_number < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -e option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("bcut", error_message);
          return 1;
        }
        break;
      }
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &block_length) ||
            block_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("bcut", error_message);
          return 1;
        }
        break;
      }
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &block_length) ||
            block_length < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("bcut", error_message);
          return 1;
        }
        ++block_length;
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

  if (kMagicNumberForEndOfFile != end_number && end_number < start_number) {
    std::ostringstream error_message;
    error_message << "End number must be equal to or greater than start number";
    sptk::PrintErrorMessage("bcut", error_message);
    return 1;
  }

  // get input file
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
      sptk::PrintErrorMessage("bcut", error_message);
      return 1;
    }
  }

  // open stream
  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("bcut", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  BinaryCutWrapper binary_cut(data_type, start_number, end_number,
                              block_length);

  if (!binary_cut.IsValid()) {
    std::ostringstream error_message;
    error_message << "Unexpected argument for the +type option";
    sptk::PrintErrorMessage("bcut", error_message);
    return 1;
  }

  if (!binary_cut.Run(&input_stream)) {
    std::ostringstream error_message;
    error_message << "Failed to cut";
    sptk::PrintErrorMessage("bcut", error_message);
    return 1;
  }

  return 0;
}
