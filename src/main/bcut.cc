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
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

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
  *stream << "       data sequence                      [stdin]" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       cut data sequence" << std::endl;
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

    // Skip data
    for (int block_index(0); block_index < start_number_; ++block_index) {
      if (!sptk::ReadStream(false, 0, 0, block_length_, &data, input_stream,
                            NULL)) {
        return false;
      }
    }

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
