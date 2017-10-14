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
#include <unordered_map>
#include <vector>

#include "SPTK/utils/int24_t.h"
#include "SPTK/utils/sptk_utils.h"
#include "SPTK/utils/uint24_t.h"

namespace {

const int kDefaultInsertPoint(0);
const int kDefaultFrameLengthOfInputData(25);
const int kDefaultFrameLengthOfInsertData(10);
const bool kDefaultOverwriteMode(false);
const char* kDefaultDataType("d");

void PrintDataType(const std::string& symbol, const std::string& type,
                   std::ostream* stream) {
  std::unordered_map<std::string, std::size_t> size({
      {"c", sizeof(int8_t)},
      {"s", sizeof(int16_t)},
      {"i3", sizeof(sptk::int24_t)},
      {"i", sizeof(int32_t)},
      {"l", sizeof(int64_t)},
      {"C", sizeof(uint8_t)},
      {"S", sizeof(uint16_t)},
      {"I3", sizeof(sptk::uint24_t)},
      {"I", sizeof(uint32_t)},
      {"L", sizeof(uint64_t)},
      {"f", sizeof(float)},
      {"d", sizeof(double)},
      {"de", sizeof(long double)},
  });
  std::ostringstream oss;
  oss << std::setw(3) << std::left << symbol;
  oss << "(" << type << ", " << size[symbol] << "byte)";
  *stream << std::setw(27) << std::left << oss.str();
}

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " merge - data merge" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       merge [ options ] file1 [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -s s  : insert point                (   int)[" << std::setw(5) << std::right << kDefaultInsertPoint             << "][ 0 <= s <= l ]" << std::endl;  // NOLINT
  *stream << "       -l l  : frame length of input data  (   int)[" << std::setw(5) << std::right << kDefaultFrameLengthOfInputData  << "][ 0 <  l <=   ]" << std::endl;  // NOLINT
  *stream << "       -n n  : order of input data         (   int)[" << std::setw(5) << std::right << "l-1"                           << "][ 0 <= n <=   ]" << std::endl;  // NOLINT
  *stream << "       -L L  : frame length of insert data (   int)[" << std::setw(5) << std::right << kDefaultFrameLengthOfInsertData << "][ 0 <  L <=   ]" << std::endl;  // NOLINT
  *stream << "       -N N  : order of insert data        (   int)[" << std::setw(5) << std::right << "L-1"                           << "][ 0 <= N <=   ]" << std::endl;  // NOLINT
  *stream << "       -w    : overwrite mode              (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultOverwriteMode) << "]" << std::endl;  // NOLINT
  *stream << "       +type : data type                           [" << std::setw(5) << std::right << kDefaultDataType                << "]" << std::endl;  // NOLINT
  *stream << "                 "; PrintDataType("c",  "char",        stream); PrintDataType("C",  "unsigned char",      stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; PrintDataType("s",  "short",       stream); PrintDataType("S",  "unsigned short",     stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; PrintDataType("i3", "int",         stream); PrintDataType("I3", "unsigned int",       stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; PrintDataType("i",  "int",         stream); PrintDataType("I",  "unsigned int",       stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; PrintDataType("l",  "long",        stream); PrintDataType("L",  "unsigned long",      stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; PrintDataType("f",  "float",       stream); PrintDataType("d",  "double",             stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; PrintDataType("de", "long double", stream);                                                    *stream << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  file1:" << std::endl;
  *stream << "       insert data sequence" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       input data sequence                         [stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       merged data sequence" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

class VectorMergeInterface {
 public:
  virtual ~VectorMergeInterface() {
  }

  virtual bool Run() = 0;
};

template <typename T>
class VectorMerge : public VectorMergeInterface {
 public:
  VectorMerge(int insert_point, int input_length, int insert_length,
              bool overwrite_mode, std::istream* input_stream,
              std::istream* insert_stream)
      : insert_point_(insert_point),
        input_length_(input_length),
        insert_length_(insert_length),
        merged_length_(overwrite_mode ? input_length
                                      : input_length + insert_length),
        input_rest_length_(merged_length_ - insert_point - insert_length),
        input_skip_length_(overwrite_mode ? insert_length : 0),
        input_stream_(input_stream),
        insert_stream_(insert_stream),
        merged_vector_(merged_length_) {
  }

  ~VectorMerge() {
  }

  virtual bool Run() {
    for (;;) {
      if (0 < insert_point_) {
        if (!sptk::ReadStream(false, 0, 0, insert_point_, &merged_vector_,
                              input_stream_)) {
          break;
        }
      }
      if (!sptk::ReadStream(false, 0, insert_point_, insert_length_,
                            &merged_vector_, insert_stream_)) {
        break;
      }
      if (0 < input_rest_length_) {
        if (!sptk::ReadStream(
                false, input_skip_length_, insert_point_ + insert_length_,
                input_rest_length_, &merged_vector_, input_stream_)) {
          break;
        }
      }
      if (!sptk::WriteStream(0, merged_length_, merged_vector_, &std::cout)) {
        return false;
      }
    }

    return (input_stream_->peek() == std::istream::traits_type::eof() &&
            insert_stream_->peek() == std::istream::traits_type::eof());
  }

 private:
  const int insert_point_;
  const int input_length_;
  const int insert_length_;
  const int merged_length_;
  const int input_rest_length_;
  const int input_skip_length_;
  std::istream* input_stream_;
  std::istream* insert_stream_;
  std::vector<T> merged_vector_;

  DISALLOW_COPY_AND_ASSIGN(VectorMerge<T>);
};

template class VectorMerge<int8_t>;
template class VectorMerge<int16_t>;
template class VectorMerge<sptk::int24_t>;
template class VectorMerge<int32_t>;
template class VectorMerge<int64_t>;
template class VectorMerge<uint8_t>;
template class VectorMerge<uint16_t>;
template class VectorMerge<sptk::uint24_t>;
template class VectorMerge<uint32_t>;
template class VectorMerge<uint64_t>;
template class VectorMerge<float>;
template class VectorMerge<double>;
template class VectorMerge<long double>;

class VectorMergeWrapper {
 public:
  VectorMergeWrapper(const std::string& data_type, int insert_point,
                     int input_length, int insert_length, bool overwrite_mode,
                     std::istream* input_stream, std::istream* insert_stream) {
    if ("c" == data_type) {
      merge_ =
          new VectorMerge<int8_t>(insert_point, input_length, insert_length,
                                  overwrite_mode, input_stream, insert_stream);
    } else if ("s" == data_type) {
      merge_ =
          new VectorMerge<int16_t>(insert_point, input_length, insert_length,
                                   overwrite_mode, input_stream, insert_stream);
    } else if ("i3" == data_type) {
      merge_ = new VectorMerge<sptk::int24_t>(insert_point, input_length,
                                              insert_length, overwrite_mode,
                                              input_stream, insert_stream);
    } else if ("i" == data_type) {
      merge_ =
          new VectorMerge<int32_t>(insert_point, input_length, insert_length,
                                   overwrite_mode, input_stream, insert_stream);
    } else if ("l" == data_type) {
      merge_ =
          new VectorMerge<int64_t>(insert_point, input_length, insert_length,
                                   overwrite_mode, input_stream, insert_stream);
    } else if ("C" == data_type) {
      merge_ =
          new VectorMerge<uint8_t>(insert_point, input_length, insert_length,
                                   overwrite_mode, input_stream, insert_stream);
    } else if ("S" == data_type) {
      merge_ = new VectorMerge<uint16_t>(insert_point, input_length,
                                         insert_length, overwrite_mode,
                                         input_stream, insert_stream);
    } else if ("I3" == data_type) {
      merge_ = new VectorMerge<sptk::uint24_t>(insert_point, input_length,
                                               insert_length, overwrite_mode,
                                               input_stream, insert_stream);
    } else if ("I" == data_type) {
      merge_ = new VectorMerge<uint32_t>(insert_point, input_length,
                                         insert_length, overwrite_mode,
                                         input_stream, insert_stream);
    } else if ("L" == data_type) {
      merge_ = new VectorMerge<uint64_t>(insert_point, input_length,
                                         insert_length, overwrite_mode,
                                         input_stream, insert_stream);
    } else if ("f" == data_type) {
      merge_ =
          new VectorMerge<float>(insert_point, input_length, insert_length,
                                 overwrite_mode, input_stream, insert_stream);
    } else if ("d" == data_type) {
      merge_ =
          new VectorMerge<double>(insert_point, input_length, insert_length,
                                  overwrite_mode, input_stream, insert_stream);
    } else if ("de" == data_type) {
      merge_ = new VectorMerge<long double>(insert_point, input_length,
                                            insert_length, overwrite_mode,
                                            input_stream, insert_stream);
    } else {
      merge_ = NULL;
    }
  }

  ~VectorMergeWrapper() {
    delete merge_;
  }

  bool IsValid() const {
    return NULL != merge_;
  }

  bool Run() const {
    return IsValid() && merge_->Run();
  }

 private:
  VectorMergeInterface* merge_;

  DISALLOW_COPY_AND_ASSIGN(VectorMergeWrapper);
};

}  // namespace

int main(int argc, char* argv[]) {
  int insert_point(kDefaultInsertPoint);
  int input_length(kDefaultFrameLengthOfInputData);
  int insert_length(kDefaultFrameLengthOfInsertData);
  bool overwrite_mode(kDefaultOverwriteMode);
  std::string data_type(kDefaultDataType);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "s:l:n:L:N:wh", NULL, NULL));
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
      case 'n': {
        if (!sptk::ConvertStringToInteger(optarg, &input_length) ||
            input_length < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -n option must be a "
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
      case 'N': {
        if (!sptk::ConvertStringToInteger(optarg, &insert_length) ||
            insert_length < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -N option must be a "
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

  // get input files
  const char* insert_file(NULL);
  const char* input_file(NULL);
  for (int i(argc - optind); 1 <= i; --i) {
    const char* arg = argv[argc - i];
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
  if (NULL == insert_file || NULL == input_file) {
    std::ostringstream error_message;
    error_message << "Two input files, file1 and infile, are required";
    sptk::PrintErrorMessage("merge", error_message);
    return 1;
  }

  // open stream for reading insert data
  std::ifstream ifs1;
  ifs1.open(insert_file, std::ios::in | std::ios::binary);
  if (ifs1.fail()) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << insert_file;
    sptk::PrintErrorMessage("merge", error_message);
    return 1;
  }
  std::istream& insert_stream(ifs1);

  // open stream for reading input data
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
                           overwrite_mode, &input_stream, &insert_stream);

  if (!merge.IsValid()) {
    std::ostringstream error_message;
    error_message << "Unexpected argument for the +type option";
    sptk::PrintErrorMessage("merge", error_message);
    return 1;
  }

  if (!merge.Run()) {
    std::ostringstream error_message;
    error_message << "Failed to merge";
    sptk::PrintErrorMessage("merge", error_message);
    return 1;
  }

  return 0;
}
