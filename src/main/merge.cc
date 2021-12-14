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

const int kDefaultInsertPoint(0);
const int kDefaultFrameLengthOfInputData(25);
const int kDefaultFrameLengthOfInsertData(10);
const bool kDefaultOverwriteMode(false);
const char* kDefaultDataType("d");

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " merge - data merge" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       merge [ options ] file1 [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -s s  : insert point                (   int)[" << std::setw(5) << std::right << kDefaultInsertPoint             << "][ 0 <= s <= l ]" << std::endl;  // NOLINT
  *stream << "       -l l  : frame length of input data  (   int)[" << std::setw(5) << std::right << kDefaultFrameLengthOfInputData  << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : order of input data         (   int)[" << std::setw(5) << std::right << "l-1"                           << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -L L  : frame length of insert data (   int)[" << std::setw(5) << std::right << kDefaultFrameLengthOfInsertData << "][ 1 <= L <=   ]" << std::endl;  // NOLINT
  *stream << "       -M M  : order of insert data        (   int)[" << std::setw(5) << std::right << "L-1"                           << "][ 0 <= M <=   ]" << std::endl;  // NOLINT
  *stream << "       -w    : overwrite mode              (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultOverwriteMode) << "]" << std::endl;  // NOLINT
  *stream << "       +type : data type                           [" << std::setw(5) << std::right << kDefaultDataType                << "]" << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("c", stream); sptk::PrintDataType("C", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("s", stream); sptk::PrintDataType("S", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("h", stream); sptk::PrintDataType("H", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("i", stream); sptk::PrintDataType("I", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("l", stream); sptk::PrintDataType("L", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("f", stream); sptk::PrintDataType("d", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("e", stream);                                   *stream << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  file1:" << std::endl;
  *stream << "       insert data sequence                (  type)" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       input data sequence                 (  type)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       merged data sequence                (  type)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

class VectorMergeInterface {
 public:
  virtual ~VectorMergeInterface() {
  }

  virtual bool Run(std::istream* input_stream, std::istream* insert_stream,
                   bool* all_merged) const = 0;
};

template <typename T>
class VectorMerge : public VectorMergeInterface {
 public:
  VectorMerge(int insert_point, int input_length, int insert_length,
              bool overwrite_mode)
      : insert_point_(insert_point),
        input_length_(input_length),
        insert_length_(insert_length),
        merged_length_(overwrite_mode ? input_length_
                                      : input_length_ + insert_length_),
        input_rest_length_(merged_length_ - insert_point_ - insert_length_),
        input_skip_length_(overwrite_mode ? insert_length_ : 0) {
  }

  ~VectorMerge() {
  }

  virtual bool Run(std::istream* input_stream, std::istream* insert_stream,
                   bool* all_merged) const {
    std::vector<T> merged_vector(merged_length_);
    for (;;) {
      if (0 < insert_point_) {
        if (!sptk::ReadStream(false, 0, 0, insert_point_, &merged_vector,
                              input_stream, NULL)) {
          break;
        }
      }
      if (!sptk::ReadStream(false, 0, insert_point_, insert_length_,
                            &merged_vector, insert_stream, NULL)) {
        break;
      }
      if (0 < input_rest_length_) {
        if (!sptk::ReadStream(
                false, input_skip_length_, insert_point_ + insert_length_,
                input_rest_length_, &merged_vector, input_stream, NULL)) {
          break;
        }
      }
      if (!sptk::WriteStream(0, merged_length_, merged_vector, &std::cout,
                             NULL)) {
        return false;
      }
    }

    if (all_merged) {
      *all_merged = (input_stream->peek() == std::istream::traits_type::eof() &&
                     insert_stream->peek() == std::istream::traits_type::eof());
    }
    return true;
  }

 private:
  const int insert_point_;
  const int input_length_;
  const int insert_length_;
  const int merged_length_;
  const int input_rest_length_;
  const int input_skip_length_;

  DISALLOW_COPY_AND_ASSIGN(VectorMerge<T>);
};

class VectorMergeWrapper {
 public:
  VectorMergeWrapper(const std::string& data_type, int insert_point,
                     int input_length, int insert_length, bool overwrite_mode)
      : merge_(NULL) {
    if ("c" == data_type) {
      merge_ = new VectorMerge<int8_t>(insert_point, input_length,
                                       insert_length, overwrite_mode);
    } else if ("s" == data_type) {
      merge_ = new VectorMerge<int16_t>(insert_point, input_length,
                                        insert_length, overwrite_mode);
    } else if ("h" == data_type) {
      merge_ = new VectorMerge<sptk::int24_t>(insert_point, input_length,
                                              insert_length, overwrite_mode);
    } else if ("i" == data_type) {
      merge_ = new VectorMerge<int32_t>(insert_point, input_length,
                                        insert_length, overwrite_mode);
    } else if ("l" == data_type) {
      merge_ = new VectorMerge<int64_t>(insert_point, input_length,
                                        insert_length, overwrite_mode);
    } else if ("C" == data_type) {
      merge_ = new VectorMerge<uint8_t>(insert_point, input_length,
                                        insert_length, overwrite_mode);
    } else if ("S" == data_type) {
      merge_ = new VectorMerge<uint16_t>(insert_point, input_length,
                                         insert_length, overwrite_mode);
    } else if ("H" == data_type) {
      merge_ = new VectorMerge<sptk::uint24_t>(insert_point, input_length,
                                               insert_length, overwrite_mode);
    } else if ("I" == data_type) {
      merge_ = new VectorMerge<uint32_t>(insert_point, input_length,
                                         insert_length, overwrite_mode);
    } else if ("L" == data_type) {
      merge_ = new VectorMerge<uint64_t>(insert_point, input_length,
                                         insert_length, overwrite_mode);
    } else if ("f" == data_type) {
      merge_ = new VectorMerge<float>(insert_point, input_length, insert_length,
                                      overwrite_mode);
    } else if ("d" == data_type) {
      merge_ = new VectorMerge<double>(insert_point, input_length,
                                       insert_length, overwrite_mode);
    } else if ("e" == data_type) {
      merge_ = new VectorMerge<long double>(insert_point, input_length,
                                            insert_length, overwrite_mode);
    }
  }

  ~VectorMergeWrapper() {
    delete merge_;
  }

  bool IsValid() const {
    return NULL != merge_;
  }

  bool Run(std::istream* input_stream, std::istream* insert_stream,
           bool* all_merged) const {
    return IsValid() && merge_->Run(input_stream, insert_stream, all_merged);
  }

 private:
  VectorMergeInterface* merge_;

  DISALLOW_COPY_AND_ASSIGN(VectorMergeWrapper);
};

}  // namespace

/**
 * @a merge [ @e option ] [ @e infile ]
 *
 * - @b -s @e int
 *   - insert point @f$(0 \le S \le L_1)@f$
 * - @b -l @e int
 *   - frame length of input data @f$(1 \le L_1)@f$
 * - @b -m @e int
 *   - order of input data @f$(0 \le L_1 - 1)@f$
 * - @b -L @e int
 *   - frame length of output data @f$(1 \le L_2)@f$
 * - @b -M @e int
 *   - order of output data @f$(0 \le L_2 - 1)@f$
 * - @b -w @e bool
 *   - overwrite mode
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
 * - @b file1 @e str
 *   - insert data sequence
 * - @b infile @e str
 *   - input data sequence
 * - @b stdout
 *   - merged data sequence
 *
 * This command merges two data sequence in a frame-by-frame manner.
 * The below figure shows the overview of the command.
 *
 * @image html merge_1.png
 *
 * Insert mode example:
 *
 * @code{.sh}
 *   echo 1 1 2 2 3 3 | x2x +as > input.s
 *   echo 4 5 6 7 | x2x +as > insert.s
 *   merge -s 2 -l 2 -L 1 +s insert.s < input.s | x2x +sa
 *   # 1, 1, 4, 2, 2, 5, 3, 3, 6
 * @endcode
 *
 * Overwrite mode example:
 *
 * @code{.sh}
 *   echo 1 1 2 2 3 3 | x2x +as > input.s
 *   echo 4 5 6 7 | x2x +as > insert.s
 *   merge -w -s 0 -l 2 -L 1 +s insert.s < input.s | x2x +sa
 *   # 4, 1, 5, 2, 6, 3
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int insert_point(kDefaultInsertPoint);
  int input_length(kDefaultFrameLengthOfInputData);
  int insert_length(kDefaultFrameLengthOfInsertData);
  bool overwrite_mode(kDefaultOverwriteMode);
  std::string data_type(kDefaultDataType);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "s:l:m:L:M:wh", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 's': {
        if (!sptk::ConvertStringToInteger(optarg, &insert_point) ||
            insert_point < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -s option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("merge", error_message);
          return 1;
        }
        break;
      }
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &input_length) ||
            input_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("merge", error_message);
          return 1;
        }
        break;
      }
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &input_length) ||
            input_length < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("merge", error_message);
          return 1;
        }
        ++input_length;
        break;
      }
      case 'L': {
        if (!sptk::ConvertStringToInteger(optarg, &insert_length) ||
            insert_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -L option must be a positive integer";
          sptk::PrintErrorMessage("merge", error_message);
          return 1;
        }
        break;
      }
      case 'M': {
        if (!sptk::ConvertStringToInteger(optarg, &insert_length) ||
            insert_length < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -M option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("merge", error_message);
          return 1;
        }
        ++insert_length;
        break;
      }
      case 'w': {
        overwrite_mode = true;
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

  if (input_length < insert_point) {
    std::ostringstream error_message;
    error_message << "Insert point must be equal to or less than input length";
    sptk::PrintErrorMessage("merge", error_message);
    return 1;
  }

  if (overwrite_mode && input_length < insert_point + insert_length) {
    std::ostringstream error_message;
    error_message << "The arguments must satisfy s + L <= l in overwrite mode";
    sptk::PrintErrorMessage("merge", error_message);
    return 1;
  }

  const char* insert_file(NULL);
  const char* input_file(NULL);
  for (int i(argc - optind); 1 <= i; --i) {
    const char* arg(argv[argc - i]);
    if (0 == std::strncmp(arg, "+", 1)) {
      const std::string str(arg);
      data_type = str.substr(1, std::string::npos);
    } else if (NULL == insert_file) {
      insert_file = arg;
    } else if (NULL == input_file) {
      input_file = arg;
    } else {
      std::ostringstream error_message;
      error_message << "Just two input files, file1 and infile, are required";
      sptk::PrintErrorMessage("merge", error_message);
      return 1;
    }
  }
  if (NULL == insert_file) {
    std::ostringstream error_message;
    error_message << "Two input files, file1 and infile, are required";
    sptk::PrintErrorMessage("merge", error_message);
    return 1;
  }

  // Open stream for reading insert data.
  std::ifstream ifs1;
  ifs1.open(insert_file, std::ios::in | std::ios::binary);
  if (ifs1.fail()) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << insert_file;
    sptk::PrintErrorMessage("merge", error_message);
    return 1;
  }
  std::istream& insert_stream(ifs1);

  // Open stream for reading input data.
  std::ifstream ifs2;
  ifs2.open(input_file, std::ios::in | std::ios::binary);
  if (ifs2.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("merge", error_message);
    return 1;
  }
  std::istream& input_stream(ifs2.fail() ? std::cin : ifs2);

  VectorMergeWrapper merge(data_type, insert_point, input_length, insert_length,
                           overwrite_mode);

  if (!merge.IsValid()) {
    std::ostringstream error_message;
    error_message << "Unexpected argument for the +type option";
    sptk::PrintErrorMessage("merge", error_message);
    return 1;
  }

  if (!merge.Run(&input_stream, &insert_stream, NULL)) {
    std::ostringstream error_message;
    error_message << "Failed to merge";
    sptk::PrintErrorMessage("merge", error_message);
    return 1;
  }

  return 0;
}
