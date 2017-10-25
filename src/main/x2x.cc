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
#include <cfloat>
#include <climits>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "SPTK/utils/int24_t.h"
#include "SPTK/utils/sptk_utils.h"
#include "SPTK/utils/uint24_t.h"

namespace {

enum NumericType {
  kUnknown = 0,
  kSignedInteger,
  kUnsignedInteger,
  kFloatingPoint,
};

enum BehaviorForOutOfRangeValue {
  kClip = 0,
  kWarn,
  kExit,
  kNumKindsOfBehavior
};

const int kBufferSize(128);
const char* kDefaultDataTypes("da");
const bool kDefaultRoundingFlag(false);
const BehaviorForOutOfRangeValue kDefaultBehaviorForOutOfRangeValue(kClip);
const int kDefaultNumColumn(1);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " x2x - data type transformation" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       x2x [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       +type : input and output data types             [" << std::setw(5) << std::right << kDefaultDataTypes << "]" << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("c", stream); sptk::PrintDataType("C", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("s", stream); sptk::PrintDataType("S", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("h", stream); sptk::PrintDataType("H", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("i", stream); sptk::PrintDataType("I", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("l", stream); sptk::PrintDataType("L", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("f", stream); sptk::PrintDataType("d", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("e", stream); sptk::PrintDataType("a", stream); *stream << std::endl;  // NOLINT
  *stream << "       -r    : rounding                        (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultRoundingFlag) << "]" << std::endl;  // NOLINT
  *stream << "       -e e  : behavior for out-of-range value (   int)[" << std::setw(5) << std::right << kDefaultBehaviorForOutOfRangeValue << "][ 0 <= e <= 2 ]" << std::endl;  // NOLINT
  *stream << "                 0 (clip the value)" << std::endl;
  *stream << "                 1 (clip the value and output warning)" << std::endl;  // NOLINT
  *stream << "                 2 (exit immediately)" << std::endl;
  *stream << "       -c c  : number of column                (   int)[" << std::setw(5) << std::right << kDefaultNumColumn                  << "][ 1 <= c <=   ]" << std::endl;  // NOLINT
  *stream << "       -f f  : print format                    (string)[" << std::setw(5) << std::right << "N/A" << "]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence                                   [stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       transformed data sequence" << std::endl;
  *stream << "  notice:" << std::endl;
  *stream << "       values of f and c are valid only if output data type is ascii" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

class DataTransformInterface {
 public:
  virtual ~DataTransformInterface() {
  }
  virtual bool Run(std::istream* input_stream) const = 0;
};

template <typename T1, typename T2>
class DataTransform : public DataTransformInterface {
 public:
  DataTransform(const std::string& print_format, int num_column,
                NumericType input_numeric_type,
                BehaviorForOutOfRangeValue behavior, bool rounding,
                bool is_ascii_input, bool is_ascii_output, T2 minimum_value = 0,
                T2 maximum_value = 0)
      : print_format_(print_format.c_str()),
        num_column_(num_column),
        input_numeric_type_(input_numeric_type),
        behavior_(behavior),
        rounding_(rounding),
        is_ascii_input_(is_ascii_input),
        is_ascii_output_(is_ascii_output),
        minimum_value_(minimum_value),
        maximum_value_(maximum_value) {
  }

  ~DataTransform() {
  }

  virtual bool Run(std::istream* input_stream) const {
    int index(0);
    for (;; ++index) {
      // read
      T1 input_data;
      if (is_ascii_input_) {
        std::string word;
        *input_stream >> word;
        if (word.empty()) break;
        try {
          input_data = std::stold(word);
        } catch (std::invalid_argument) {
          return false;
        }
      } else {
        if (!sptk::ReadStream(&input_data, input_stream)) {
          break;
        }
      }

      // convert
      T2 output_data(input_data);
      bool is_clipped(false);
      {
        // clipping
        if (minimum_value_ < maximum_value_) {
          if (kSignedInteger == input_numeric_type_) {
            if (static_cast<int64_t>(input_data) <
                static_cast<int64_t>(minimum_value_)) {
              output_data = minimum_value_;
              is_clipped = true;
            } else if (static_cast<int64_t>(maximum_value_) <
                       static_cast<int64_t>(input_data)) {
              output_data = maximum_value_;
              is_clipped = true;
            }
          } else if (kUnsignedInteger == input_numeric_type_) {
            if (static_cast<uint64_t>(input_data) <
                static_cast<uint64_t>(minimum_value_)) {
              output_data = minimum_value_;
              is_clipped = true;
            } else if (static_cast<uint64_t>(maximum_value_) <
                       static_cast<uint64_t>(input_data)) {
              output_data = maximum_value_;
              is_clipped = true;
            }
          } else if (kFloatingPoint == input_numeric_type_) {
            if (static_cast<long double>(input_data) <
                static_cast<long double>(minimum_value_)) {
              output_data = minimum_value_;
              is_clipped = true;
            } else if (static_cast<long double>(maximum_value_) <
                       static_cast<long double>(input_data)) {
              output_data = maximum_value_;
              is_clipped = true;
            }
          }
        }

        // rounding
        if (rounding_ && !is_clipped) {
          if (0.0 < input_data) {
            output_data = static_cast<T2>(input_data + 0.5);
          } else {
            output_data = static_cast<T2>(input_data - 0.5);
          }
        }
      }

      if (is_clipped && (kWarn == behavior_ || kExit == behavior_)) {
        std::ostringstream error_message;
        error_message << index << "th data is over the range of output type";
        sptk::PrintErrorMessage("x2x", error_message);
        if (kExit == behavior_) return false;
      }

      // write
      if (is_ascii_output_) {
        char buffer[kBufferSize];
        if (3 == sizeof(T2)) {  // int24_t and uint24_t
          if (std::snprintf(buffer, sizeof(buffer), print_format_,
                            static_cast<int>(output_data)) < 0) {
            return false;
          }
        } else {
          if (std::snprintf(buffer, sizeof(buffer), print_format_,
                            output_data) < 0) {
            return false;
          }
        }
        std::cout << buffer;
        if (0 == index % num_column_) {
          std::cout << std::endl;
        } else {
          std::cout << "\t";
        }
      } else {
        if (!sptk::WriteStream(output_data, &std::cout)) {
          return false;
        }
      }
    }

    if (is_ascii_output_ && 0 != index % num_column_) {
      std::cout << std::endl;
    }

    return true;
  }

 private:
  const char* print_format_;
  const int num_column_;
  const NumericType input_numeric_type_;
  const BehaviorForOutOfRangeValue behavior_;
  const bool rounding_;
  const bool is_ascii_input_;
  const bool is_ascii_output_;
  const T2 minimum_value_;
  const T2 maximum_value_;

  DataTransform<T1, T2>(const DataTransform<T1, T2>&);
  void operator=(const DataTransform<T1, T2>&);
};

class DataTransformWrapper {
 public:
  DataTransformWrapper(const std::string& input_data_type,
                       const std::string& output_data_type,
                       const std::string& given_print_format, int num_column,
                       BehaviorForOutOfRangeValue behavior,
                       bool given_rounding_flag)
      : data_transform_(NULL) {
    std::string print_format(given_print_format);
    if (print_format.empty() && "a" == output_data_type) {
      if ("c" == input_data_type || "s" == input_data_type ||
          "h" == input_data_type || "i" == input_data_type) {
        print_format = "%d";
      } else if ("l" == input_data_type) {
        print_format = "%lld";
      } else if ("C" == input_data_type || "S" == input_data_type ||
                 "H" == input_data_type || "I" == input_data_type) {
        print_format = "%u";
      } else if ("L" == input_data_type) {
        print_format = "%llu";
      } else if ("f" == input_data_type || "d" == input_data_type) {
        print_format = "%g";
      } else if ("e" == input_data_type || "a" == input_data_type) {
        print_format = "%Lg";
      }
    }

    NumericType input_numeric_type(kUnknown);
    if ("c" == input_data_type || "s" == input_data_type ||
        "h" == input_data_type || "i" == input_data_type ||
        "l" == input_data_type) {
      input_numeric_type = kSignedInteger;
    } else if ("C" == input_data_type || "S" == input_data_type ||
               "H" == input_data_type || "I" == input_data_type ||
               "L" == input_data_type) {
      input_numeric_type = kUnsignedInteger;
    } else if ("f" == input_data_type || "d" == input_data_type ||
               "e" == input_data_type || "a" == input_data_type) {
      input_numeric_type = kFloatingPoint;
    }

    bool rounding(false);
    if (("f" == input_data_type || "d" == input_data_type ||
         "e" == input_data_type || "a" == input_data_type) &&
        ("c" == output_data_type || "C" == output_data_type ||
         "s" == output_data_type || "S" == output_data_type ||
         "h" == output_data_type || "H" == output_data_type ||
         "i" == output_data_type || "I" == output_data_type ||
         "l" == output_data_type || "L" == output_data_type)) {
      rounding = given_rounding_flag;
    }

    const bool is_ascii_input("a" == input_data_type);
    const bool is_ascii_output("a" == output_data_type);

    // c -> *
    if ("c" == input_data_type && "c" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, int8_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("c" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, int16_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("c" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, sptk::int24_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(0), sptk::int24_t(0));
    } else if ("c" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, int32_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("c" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, int64_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("c" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, uint8_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, SCHAR_MAX);
    } else if ("c" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, uint16_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, SCHAR_MAX);
    } else if ("c" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(SCHAR_MAX));
    } else if ("c" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, uint32_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, SCHAR_MAX);
    } else if ("c" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, uint64_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, SCHAR_MAX);
    } else if ("c" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, float>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("c" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, double>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("c" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, long double>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("c" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, int8_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    }

    // s -> *
    else if ("s" == input_data_type && "c" == output_data_type) {  // NOLINT
      data_transform_ = new DataTransform<int16_t, int8_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, SCHAR_MIN, SCHAR_MAX);
    } else if ("s" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, int16_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("s" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, sptk::int24_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(0), sptk::int24_t(0));
    } else if ("s" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, int32_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("s" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, int64_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("s" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, uint8_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("s" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, uint16_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, SHRT_MAX);
    } else if ("s" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(SHRT_MAX));
    } else if ("s" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, uint32_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, SHRT_MAX);
    } else if ("s" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, uint64_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, SHRT_MAX);
    } else if ("s" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, float>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("s" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, double>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("s" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, long double>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("s" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, int16_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    }

    // h -> *
    else if ("h" == input_data_type && "c" == output_data_type) {  // NOLINT
      data_transform_ = new DataTransform<sptk::int24_t, int8_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, SCHAR_MIN, SCHAR_MAX);
    } else if ("h" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, int16_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, SHRT_MIN, SHRT_MAX);
    } else if ("h" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, sptk::int24_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(0), sptk::int24_t(0));
    } else if ("h" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, int32_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("h" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, int64_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("h" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, uint8_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("h" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, uint16_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, USHRT_MAX);
    } else if ("h" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(sptk::INT24_MAX));
    } else if ("h" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, uint32_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, sptk::INT24_MAX);
    } else if ("h" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, uint64_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, sptk::INT24_MAX);
    } else if ("h" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, float>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("h" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, double>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("h" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, long double>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("h" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, sptk::int24_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(0), sptk::int24_t(0));
    }

    // i -> *
    else if ("i" == input_data_type && "c" == output_data_type) {  // NOLINT
      data_transform_ = new DataTransform<int32_t, int8_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, SCHAR_MIN, SCHAR_MAX);
    } else if ("i" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, int16_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, SHRT_MIN, SHRT_MAX);
    } else if ("i" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, sptk::int24_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(sptk::INT24_MIN),
          sptk::int24_t(sptk::INT24_MAX));
    } else if ("i" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, int32_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("i" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, int64_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("i" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, uint8_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("i" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, uint16_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, USHRT_MAX);
    } else if ("i" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(sptk::UINT24_MAX));
    } else if ("i" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, uint32_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, INT_MAX);
    } else if ("i" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, uint64_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, INT_MAX);
    } else if ("i" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, float>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("i" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, double>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("i" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, long double>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("i" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, int32_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    }

    // l -> *
    else if ("l" == input_data_type && "c" == output_data_type) {  // NOLINT
      data_transform_ = new DataTransform<int64_t, int8_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, SCHAR_MIN, SCHAR_MAX);
    } else if ("l" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, int16_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, SHRT_MIN, SHRT_MAX);
    } else if ("l" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, sptk::int24_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(sptk::INT24_MIN),
          sptk::int24_t(sptk::INT24_MAX));
    } else if ("l" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, int32_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, INT_MIN, INT_MAX);
    } else if ("l" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, int64_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("l" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, uint8_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("l" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, uint16_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, USHRT_MAX);
    } else if ("l" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(sptk::UINT24_MAX));
    } else if ("l" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, uint32_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, UINT_MAX);
    } else if ("l" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, uint64_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, LLONG_MAX);
    } else if ("l" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, float>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("l" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, double>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("l" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, long double>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("l" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, int64_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    }

    // C -> *
    else if ("C" == input_data_type && "c" == output_data_type) {  // NOLINT
      data_transform_ = new DataTransform<uint8_t, int8_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, SCHAR_MAX);
    } else if ("C" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, int16_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("C" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, sptk::int24_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(0), sptk::int24_t(0));
    } else if ("C" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, int32_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("C" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, int64_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("C" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, uint8_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("C" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, uint16_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("C" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(0));
    } else if ("C" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, uint32_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("C" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, uint64_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("C" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, float>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("C" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, double>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("C" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, long double>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("C" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, uint8_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    }

    // S -> *
    else if ("S" == input_data_type && "c" == output_data_type) {  // NOLINT
      data_transform_ = new DataTransform<uint16_t, int8_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, SCHAR_MAX);
    } else if ("S" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, int16_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, SHRT_MAX);
    } else if ("S" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, sptk::int24_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(0), sptk::int24_t(0));
    } else if ("S" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, int32_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("S" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, int64_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("S" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, uint8_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("S" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, uint16_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("S" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(0));
    } else if ("S" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, uint32_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("S" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, uint64_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("S" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, float>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("S" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, double>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("S" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, long double>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("S" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, uint16_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    }

    // H -> *
    else if ("H" == input_data_type && "c" == output_data_type) {  // NOLINT
      data_transform_ = new DataTransform<sptk::uint24_t, int8_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, SCHAR_MAX);
    } else if ("H" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, int16_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, SHRT_MAX);
    } else if ("H" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, sptk::int24_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(0),
          sptk::int24_t(sptk::INT24_MAX));
    } else if ("H" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, int32_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("H" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, int64_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("H" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, uint8_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("H" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, uint16_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, USHRT_MAX);
    } else if ("H" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(0));
    } else if ("H" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, uint32_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("H" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, uint64_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("H" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, float>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("H" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, double>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("H" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, long double>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("H" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(0));
    }

    // I -> *
    else if ("I" == input_data_type && "c" == output_data_type) {  // NOLINT
      data_transform_ = new DataTransform<uint32_t, int8_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, SCHAR_MAX);
    } else if ("I" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, int16_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, SHRT_MAX);
    } else if ("I" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, sptk::int24_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(0),
          sptk::int24_t(sptk::INT24_MAX));
    } else if ("I" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, int32_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, INT_MAX);
    } else if ("I" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, int64_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("I" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, uint8_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("I" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, uint16_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, USHRT_MAX);
    } else if ("I" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(sptk::UINT24_MAX));
    } else if ("I" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, uint32_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("I" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, uint64_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("I" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, float>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("I" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, double>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("I" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, long double>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("I" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, uint32_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    }

    // L -> *
    else if ("L" == input_data_type && "c" == output_data_type) {  // NOLINT
      data_transform_ = new DataTransform<uint64_t, int8_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, SCHAR_MAX);
    } else if ("L" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, int16_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, SHRT_MAX);
    } else if ("L" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, sptk::int24_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(0),
          sptk::int24_t(sptk::INT24_MAX));
    } else if ("L" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, int32_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, INT_MAX);
    } else if ("L" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, int64_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, LLONG_MAX);
    } else if ("L" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, uint8_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("L" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, uint16_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, USHRT_MAX);
    } else if ("L" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(sptk::UINT24_MAX));
    } else if ("L" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, uint32_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, UINT_MAX);
    } else if ("L" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, uint64_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("L" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, float>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("L" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, double>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("L" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, long double>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("L" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, uint64_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    }

    // f -> *
    else if ("f" == input_data_type && "c" == output_data_type) {  // NOLINT
      data_transform_ = new DataTransform<float, int8_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, SCHAR_MIN, SCHAR_MAX);
    } else if ("f" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<float, int16_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, SHRT_MIN, SHRT_MAX);
    } else if ("f" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<float, sptk::int24_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(sptk::INT24_MIN),
          sptk::int24_t(sptk::INT24_MAX));
    } else if ("f" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<float, int32_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, INT_MIN, INT_MAX);
    } else if ("f" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<float, int64_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, LLONG_MIN, LLONG_MAX);
    } else if ("f" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<float, uint8_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("f" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<float, uint16_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, USHRT_MAX);
    } else if ("f" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<float, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(sptk::UINT24_MAX));
    } else if ("f" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<float, uint32_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, UINT_MAX);
    } else if ("f" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<float, uint64_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, ULLONG_MAX);
    } else if ("f" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<float, float>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("f" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<float, double>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("f" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<float, long double>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("f" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<float, float>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    }

    // d -> *
    else if ("d" == input_data_type && "c" == output_data_type) {  // NOLINT
      data_transform_ = new DataTransform<double, int8_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, SCHAR_MIN, SCHAR_MAX);
    } else if ("d" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<double, int16_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, SHRT_MIN, SHRT_MAX);
    } else if ("d" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<double, sptk::int24_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(sptk::INT24_MIN),
          sptk::int24_t(sptk::INT24_MAX));
    } else if ("d" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<double, int32_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, INT_MIN, INT_MAX);
    } else if ("d" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<double, int64_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, LLONG_MIN, LLONG_MAX);
    } else if ("d" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<double, uint8_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("d" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<double, uint16_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, USHRT_MAX);
    } else if ("d" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<double, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(sptk::UINT24_MAX));
    } else if ("d" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<double, uint32_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, UINT_MAX);
    } else if ("d" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<double, uint64_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, ULLONG_MAX);
    } else if ("d" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<double, float>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, -FLT_MAX, FLT_MAX);
    } else if ("d" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<double, double>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("d" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<double, long double>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("d" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<double, double>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    }

    // e -> *
    else if ("e" == input_data_type && "c" == output_data_type) {  // NOLINT
      data_transform_ = new DataTransform<long double, int8_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, SCHAR_MIN, SCHAR_MAX);
    } else if ("e" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<long double, int16_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, SHRT_MIN, SHRT_MAX);
    } else if ("e" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<long double, sptk::int24_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(sptk::INT24_MIN),
          sptk::int24_t(sptk::INT24_MAX));
    } else if ("e" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<long double, int32_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, INT_MIN, INT_MAX);
    } else if ("e" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<long double, int64_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, LLONG_MIN, LLONG_MAX);
    } else if ("e" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<long double, uint8_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("e" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<long double, uint16_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, USHRT_MAX);
    } else if ("e" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<long double, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(sptk::UINT24_MAX));
    } else if ("e" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<long double, uint32_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, UINT_MAX);
    } else if ("e" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<long double, uint64_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, ULLONG_MAX);
    } else if ("e" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<long double, float>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, -FLT_MAX, FLT_MAX);
    } else if ("e" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<long double, double>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, -DBL_MAX, DBL_MAX);
    } else if ("e" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<long double, long double>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("e" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<long double, long double>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    }

    // a -> *
    else if ("a" == input_data_type && "c" == output_data_type) {  // NOLINT
      data_transform_ = new DataTransform<long double, int8_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, SCHAR_MIN, SCHAR_MAX);
    } else if ("a" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<long double, int16_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, SHRT_MIN, SHRT_MAX);
    } else if ("a" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<long double, sptk::int24_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(sptk::INT24_MIN),
          sptk::int24_t(sptk::INT24_MAX));
    } else if ("a" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<long double, int32_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, INT_MIN, INT_MAX);
    } else if ("a" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<long double, int64_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, LLONG_MIN, LLONG_MAX);
    } else if ("a" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<long double, uint8_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("a" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<long double, uint16_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, USHRT_MAX);
    } else if ("a" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<long double, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(sptk::UINT24_MAX));
    } else if ("a" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<long double, uint32_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, UINT_MAX);
    } else if ("a" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<long double, uint64_t>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, 0, ULLONG_MAX);
    } else if ("a" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<long double, float>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, -FLT_MAX, FLT_MAX);
    } else if ("a" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<long double, double>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output, -DBL_MAX, DBL_MAX);
    } else if ("a" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<long double, long double>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("a" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<long double, long double>(
          print_format, num_column, input_numeric_type, behavior, rounding,
          is_ascii_input, is_ascii_output);
    }
  }

  ~DataTransformWrapper() {
    delete data_transform_;
  }

  bool IsValid() const {
    return NULL != data_transform_;
  }

  bool Run(std::istream* input_stream) const {
    return IsValid() && data_transform_->Run(input_stream);
  }

 private:
  DataTransformInterface* data_transform_;

  DISALLOW_COPY_AND_ASSIGN(DataTransformWrapper);
};

}  // namespace

int main(int argc, char* argv[]) {
  bool rounding_flag(kDefaultRoundingFlag);
  BehaviorForOutOfRangeValue behavior(kDefaultBehaviorForOutOfRangeValue);
  int num_column(kDefaultNumColumn);
  std::string print_format("");
  std::string data_types(kDefaultDataTypes);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "re:c:f:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'r': {
        rounding_flag = true;
        break;
      }
      case 'e': {
        const int min(0);
        const int max(static_cast<int>(kNumKindsOfBehavior) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -e option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("x2x", error_message);
          return 1;
        }
        behavior = static_cast<BehaviorForOutOfRangeValue>(tmp);
        break;
      }
      case 'c': {
        if (!sptk::ConvertStringToInteger(optarg, &num_column) ||
            num_column <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -c option must be a positive integer";
          sptk::PrintErrorMessage("x2x", error_message);
          return 1;
        }
        break;
      }
      case 'f': {
        print_format = optarg;
        if ("%" != print_format.substr(0, 1)) {
          std::ostringstream error_message;
          error_message << "The argument for the -f option must be begin with "
                        << "%";
          sptk::PrintErrorMessage("x2x", error_message);
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

  // get input file
  const char* input_file(NULL);
  for (int i(argc - optind); 1 <= i; --i) {
    const char* arg(argv[argc - i]);
    if (0 == std::strncmp(arg, "+", 1)) {
      const std::string str(arg);
      data_types = str.substr(1, std::string::npos);
      if (2 != data_types.size()) {
        std::ostringstream error_message;
        error_message << "The +type option must be two characters";
        sptk::PrintErrorMessage("x2x", error_message);
        return 1;
      }
    } else if (NULL == input_file) {
      input_file = arg;
    } else {
      std::ostringstream error_message;
      error_message << "Too many input files";
      sptk::PrintErrorMessage("x2x", error_message);
      return 1;
    }
  }

  // open stream
  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("x2x", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  const std::string input_data_type(data_types.substr(0, 1));
  const std::string output_data_type(data_types.substr(1, 1));
  DataTransformWrapper data_transform(input_data_type, output_data_type,
                                      print_format, num_column, behavior,
                                      rounding_flag);

  if (!data_transform.IsValid()) {
    std::ostringstream error_message;
    error_message << "Unexpected argument for the +type option";
    sptk::PrintErrorMessage("x2x", error_message);
    return 1;
  }

  if (!data_transform.Run(&input_stream)) {
    std::ostringstream error_message;
    error_message << "Failed to transform";
    sptk::PrintErrorMessage("x2x", error_message);
    return 1;
  }

  return 0;
}
