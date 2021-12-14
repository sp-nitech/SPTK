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

#include <cfloat>     // DBL_MAX, FLT_MAX
#include <climits>    // INT_MIN, INT_MAX, SCHAR_MIN, SCHAR_MAX, etc.
#include <cstdint>    // int8_t, int16_t, int32_t, int64_t, etc.
#include <cstring>    // std::strncmp
#include <fstream>    // std::ifstream
#include <iomanip>    // std::setw
#include <iostream>   // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>    // std::ostringstream
#include <stdexcept>  // std::invalid_argument
#include <string>     // std::stold, std::string

#include "Getopt/getoptwin.h"
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

enum WarningType { kIgnore = 0, kWarn, kExit, kNumWarningTypes };

const int kBufferSize(128);
const char* kDefaultDataTypes("da");
const bool kDefaultRoundingFlag(false);
const WarningType kDefaultWarningType(kExit);
const int kDefaultNumColumn(1);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " x2x - data type transformation" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       x2x [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       +type : input and output data types                [" << std::setw(5) << std::right << kDefaultDataTypes  << "]" << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("c", stream); sptk::PrintDataType("C", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("s", stream); sptk::PrintDataType("S", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("h", stream); sptk::PrintDataType("H", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("i", stream); sptk::PrintDataType("I", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("l", stream); sptk::PrintDataType("L", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("f", stream); sptk::PrintDataType("d", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("e", stream); sptk::PrintDataType("a", stream); *stream << std::endl;  // NOLINT
  *stream << "       -r    : rounding                           (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultRoundingFlag) << "]" << std::endl;  // NOLINT
  *stream << "       -e e  : warning type of out-of-range value (   int)[" << std::setw(5) << std::right << kDefaultWarningType << "][ 0 <= e <= 2 ]" << std::endl;  // NOLINT
  *stream << "                 0 (no warning)" << std::endl;
  *stream << "                 1 (output the index to stderr)" << std::endl;
  *stream << "                 2 (output the index to stderr" << std::endl;
  *stream << "                    and exit immediately)" << std::endl;
  *stream << "       -c c  : number of columns                  (   int)[" << std::setw(5) << std::right << kDefaultNumColumn   << "][ 1 <= c <=   ]" << std::endl;  // NOLINT
  *stream << "       -f f  : print format                       (string)[" << std::setw(5) << std::right << "N/A"               << "]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence                                      [stdin]" << std::endl;  // NOLINT
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
                NumericType input_numeric_type, WarningType warning_type,
                bool rounding, bool is_ascii_input, bool is_ascii_output,
                T2 minimum_value = 0, T2 maximum_value = 0)
      : print_format_(print_format),
        num_column_(num_column),
        input_numeric_type_(input_numeric_type),
        warning_type_(warning_type),
        rounding_(rounding),
        is_ascii_input_(is_ascii_input),
        is_ascii_output_(is_ascii_output),
        minimum_value_(minimum_value),
        maximum_value_(maximum_value) {
  }

  ~DataTransform() {
  }

  virtual bool Run(std::istream* input_stream) const {
    char buffer[kBufferSize];
    int index(0);
    for (;; ++index) {
      // Read.
      T1 input_data;
      if (is_ascii_input_) {
        std::string word;
        *input_stream >> word;
        if (word.empty()) break;
        try {
          input_data = std::stold(word);
        } catch (std::invalid_argument&) {
          return false;
        }
      } else {
        if (!sptk::ReadStream(&input_data, input_stream)) {
          break;
        }
      }

      // Convert.
      T2 output_data(input_data);

      bool is_clipped(false);
      {
        // Clipping.
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

        // Rounding.
        if (rounding_ && !is_clipped) {
          if (0.0 < input_data) {
            output_data = static_cast<T2>(input_data + 0.5);
          } else {
            output_data = static_cast<T2>(input_data - 0.5);
          }
        }
      }

      if (is_clipped && kIgnore != warning_type_) {
        std::ostringstream error_message;
        error_message << index << "th data is over the range of output type";
        sptk::PrintErrorMessage("x2x", error_message);
        if (kExit == warning_type_) return false;
      }

      // Write output.
      if (is_ascii_output_) {
        if (!sptk::SnPrintf(output_data, print_format_, sizeof(buffer),
                            buffer)) {
          return false;
        }
        std::cout << buffer;
        if (0 == (index + 1) % num_column_) {
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
  const std::string print_format_;
  const int num_column_;
  const NumericType input_numeric_type_;
  const WarningType warning_type_;
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
                       WarningType warning_type, bool given_rounding_flag)
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
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("c" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, int16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("c" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, sptk::int24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(0), sptk::int24_t(0));
    } else if ("c" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, int32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("c" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, int64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("c" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, uint8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, SCHAR_MAX);
    } else if ("c" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, uint16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, SCHAR_MAX);
    } else if ("c" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(SCHAR_MAX));
    } else if ("c" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, uint32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, SCHAR_MAX);
    } else if ("c" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, uint64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, SCHAR_MAX);
    } else if ("c" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, float>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("c" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("c" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, long double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("c" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, int8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    }

    // s -> *
    if ("s" == input_data_type && "c" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, int8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, SCHAR_MIN, SCHAR_MAX);
    } else if ("s" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, int16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("s" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, sptk::int24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(0), sptk::int24_t(0));
    } else if ("s" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, int32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("s" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, int64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("s" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, uint8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("s" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, uint16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, SHRT_MAX);
    } else if ("s" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(SHRT_MAX));
    } else if ("s" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, uint32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, SHRT_MAX);
    } else if ("s" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, uint64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, SHRT_MAX);
    } else if ("s" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, float>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("s" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("s" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, long double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("s" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, int16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    }

    // h -> *
    if ("h" == input_data_type && "c" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, int8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, SCHAR_MIN, SCHAR_MAX);
    } else if ("h" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, int16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, SHRT_MIN, SHRT_MAX);
    } else if ("h" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, sptk::int24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(0), sptk::int24_t(0));
    } else if ("h" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, int32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("h" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, int64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("h" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, uint8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("h" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, uint16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, USHRT_MAX);
    } else if ("h" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(sptk::INT24_MAX));
    } else if ("h" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, uint32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, sptk::INT24_MAX);
    } else if ("h" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, uint64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, sptk::INT24_MAX);
    } else if ("h" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, float>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("h" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("h" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, long double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("h" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, sptk::int24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(0), sptk::int24_t(0));
    }

    // i -> *
    if ("i" == input_data_type && "c" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, int8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, SCHAR_MIN, SCHAR_MAX);
    } else if ("i" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, int16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, SHRT_MIN, SHRT_MAX);
    } else if ("i" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, sptk::int24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(sptk::INT24_MIN),
          sptk::int24_t(sptk::INT24_MAX));
    } else if ("i" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, int32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("i" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, int64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("i" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, uint8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("i" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, uint16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, USHRT_MAX);
    } else if ("i" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(sptk::UINT24_MAX));
    } else if ("i" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, uint32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, INT_MAX);
    } else if ("i" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, uint64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, INT_MAX);
    } else if ("i" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, float>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("i" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("i" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, long double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("i" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, int32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    }

    // l -> *
    if ("l" == input_data_type && "c" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, int8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, SCHAR_MIN, SCHAR_MAX);
    } else if ("l" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, int16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, SHRT_MIN, SHRT_MAX);
    } else if ("l" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, sptk::int24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(sptk::INT24_MIN),
          sptk::int24_t(sptk::INT24_MAX));
    } else if ("l" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, int32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, INT_MIN, INT_MAX);
    } else if ("l" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, int64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("l" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, uint8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("l" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, uint16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, USHRT_MAX);
    } else if ("l" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(sptk::UINT24_MAX));
    } else if ("l" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, uint32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UINT_MAX);
    } else if ("l" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, uint64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, LLONG_MAX);
    } else if ("l" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, float>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("l" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("l" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, long double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("l" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, int64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    }

    // C -> *
    if ("C" == input_data_type && "c" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, int8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, SCHAR_MAX);
    } else if ("C" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, int16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("C" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, sptk::int24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(0), sptk::int24_t(0));
    } else if ("C" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, int32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("C" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, int64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("C" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, uint8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("C" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, uint16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("C" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(0));
    } else if ("C" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, uint32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("C" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, uint64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("C" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, float>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("C" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("C" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, long double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("C" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, uint8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    }

    // S -> *
    if ("S" == input_data_type && "c" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, int8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, SCHAR_MAX);
    } else if ("S" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, int16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, SHRT_MAX);
    } else if ("S" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, sptk::int24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(0), sptk::int24_t(0));
    } else if ("S" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, int32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("S" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, int64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("S" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, uint8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("S" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, uint16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("S" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(0));
    } else if ("S" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, uint32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("S" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, uint64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("S" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, float>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("S" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("S" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, long double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("S" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, uint16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    }

    // H -> *
    if ("H" == input_data_type && "c" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, int8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, SCHAR_MAX);
    } else if ("H" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, int16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, SHRT_MAX);
    } else if ("H" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, sptk::int24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(0),
          sptk::int24_t(sptk::INT24_MAX));
    } else if ("H" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, int32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("H" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, int64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("H" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, uint8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("H" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, uint16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, USHRT_MAX);
    } else if ("H" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(0));
    } else if ("H" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, uint32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("H" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, uint64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("H" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, float>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("H" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("H" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, long double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("H" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(0));
    }

    // I -> *
    if ("I" == input_data_type && "c" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, int8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, SCHAR_MAX);
    } else if ("I" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, int16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, SHRT_MAX);
    } else if ("I" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, sptk::int24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(0),
          sptk::int24_t(sptk::INT24_MAX));
    } else if ("I" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, int32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, INT_MAX);
    } else if ("I" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, int64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("I" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, uint8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("I" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, uint16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, USHRT_MAX);
    } else if ("I" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(sptk::UINT24_MAX));
    } else if ("I" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, uint32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("I" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, uint64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("I" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, float>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("I" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("I" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, long double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("I" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, uint32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    }

    // L -> *
    if ("L" == input_data_type && "c" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, int8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, SCHAR_MAX);
    } else if ("L" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, int16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, SHRT_MAX);
    } else if ("L" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, sptk::int24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(0),
          sptk::int24_t(sptk::INT24_MAX));
    } else if ("L" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, int32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, INT_MAX);
    } else if ("L" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, int64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, LLONG_MAX);
    } else if ("L" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, uint8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("L" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, uint16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, USHRT_MAX);
    } else if ("L" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(sptk::UINT24_MAX));
    } else if ("L" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, uint32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UINT_MAX);
    } else if ("L" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, uint64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("L" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, float>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("L" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("L" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, long double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("L" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, uint64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    }

    // f -> *
    if ("f" == input_data_type && "c" == output_data_type) {
      data_transform_ = new DataTransform<float, int8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, SCHAR_MIN, SCHAR_MAX);
    } else if ("f" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<float, int16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, SHRT_MIN, SHRT_MAX);
    } else if ("f" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<float, sptk::int24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(sptk::INT24_MIN),
          sptk::int24_t(sptk::INT24_MAX));
    } else if ("f" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<float, int32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, INT_MIN, INT_MAX);
    } else if ("f" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<float, int64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, LLONG_MIN, LLONG_MAX);
    } else if ("f" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<float, uint8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("f" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<float, uint16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, USHRT_MAX);
    } else if ("f" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<float, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(sptk::UINT24_MAX));
    } else if ("f" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<float, uint32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UINT_MAX);
    } else if ("f" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<float, uint64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, ULLONG_MAX);
    } else if ("f" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<float, float>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("f" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<float, double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("f" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<float, long double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("f" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<float, float>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    }

    // d -> *
    if ("d" == input_data_type && "c" == output_data_type) {
      data_transform_ = new DataTransform<double, int8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, SCHAR_MIN, SCHAR_MAX);
    } else if ("d" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<double, int16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, SHRT_MIN, SHRT_MAX);
    } else if ("d" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<double, sptk::int24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(sptk::INT24_MIN),
          sptk::int24_t(sptk::INT24_MAX));
    } else if ("d" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<double, int32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, INT_MIN, INT_MAX);
    } else if ("d" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<double, int64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, LLONG_MIN, LLONG_MAX);
    } else if ("d" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<double, uint8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("d" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<double, uint16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, USHRT_MAX);
    } else if ("d" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<double, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(sptk::UINT24_MAX));
    } else if ("d" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<double, uint32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UINT_MAX);
    } else if ("d" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<double, uint64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, ULLONG_MAX);
    } else if ("d" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<double, float>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, -FLT_MAX, FLT_MAX);
    } else if ("d" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<double, double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("d" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<double, long double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("d" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<double, double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    }

    // e -> *
    if ("e" == input_data_type && "c" == output_data_type) {
      data_transform_ = new DataTransform<long double, int8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, SCHAR_MIN, SCHAR_MAX);
    } else if ("e" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<long double, int16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, SHRT_MIN, SHRT_MAX);
    } else if ("e" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<long double, sptk::int24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(sptk::INT24_MIN),
          sptk::int24_t(sptk::INT24_MAX));
    } else if ("e" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<long double, int32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, INT_MIN, INT_MAX);
    } else if ("e" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<long double, int64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, LLONG_MIN, LLONG_MAX);
    } else if ("e" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<long double, uint8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("e" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<long double, uint16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, USHRT_MAX);
    } else if ("e" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<long double, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(sptk::UINT24_MAX));
    } else if ("e" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<long double, uint32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UINT_MAX);
    } else if ("e" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<long double, uint64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, ULLONG_MAX);
    } else if ("e" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<long double, float>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, -FLT_MAX, FLT_MAX);
    } else if ("e" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<long double, double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, -DBL_MAX, DBL_MAX);
    } else if ("e" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<long double, long double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("e" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<long double, long double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    }

    // a -> *
    if ("a" == input_data_type && "c" == output_data_type) {
      data_transform_ = new DataTransform<long double, int8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, SCHAR_MIN, SCHAR_MAX);
    } else if ("a" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<long double, int16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, SHRT_MIN, SHRT_MAX);
    } else if ("a" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<long double, sptk::int24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(sptk::INT24_MIN),
          sptk::int24_t(sptk::INT24_MAX));
    } else if ("a" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<long double, int32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, INT_MIN, INT_MAX);
    } else if ("a" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<long double, int64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, LLONG_MIN, LLONG_MAX);
    } else if ("a" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<long double, uint8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("a" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<long double, uint16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, USHRT_MAX);
    } else if ("a" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<long double, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(sptk::UINT24_MAX));
    } else if ("a" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<long double, uint32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UINT_MAX);
    } else if ("a" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<long double, uint64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, ULLONG_MAX);
    } else if ("a" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<long double, float>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, -FLT_MAX, FLT_MAX);
    } else if ("a" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<long double, double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, -DBL_MAX, DBL_MAX);
    } else if ("a" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<long double, long double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("a" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<long double, long double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
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

/**
 * @a x2x [ @e option ] [ @e infile ]
 *
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
 * - @b -r @e bool
 *   - rounding
 * - @b -e @e int
 *   - warning type for out-of-range value
 *     \arg @c 0 no warning
 *     \arg @c 1 output index
 *     \arg @c 2 output index and exit immediately
 * - @b -c @e int
 *   - number of columns
 * - @b -f @e str
 *   - print format
 * - @b infile @e str
 *   - double-type data sequence
 * - @b stdout
 *   - double-type transformed data sequence
 *
 * @code{.sh}
 *   ramp -l 4 | x2x +da
 *   # 0
 *   # 1
 *   # 2
 *   # 3
 *   ramp -l 4 | x2x +da -c 2
 *   # 0       1
 *   # 2       3
 *   ramp -l 4 | sopr -a 0.5 | x2x +dc -r | x2x +ca -c 2
 *   # 1       2
 *   # 3       4
 *   ramp -l 4 | x2x +da -c 2 -f %.1f
 *   # 0.0     1.0
 *   # 2.0     3.0
 *   echo -1 1000 | x2x +aC -e 0 | x2x +Ca
 *   # 0
 *   # 255
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  bool rounding_flag(kDefaultRoundingFlag);
  WarningType warning_type(kDefaultWarningType);
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
        const int max(static_cast<int>(kNumWarningTypes) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -e option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("x2x", error_message);
          return 1;
        }
        warning_type = static_cast<WarningType>(tmp);
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
          error_message
              << "The argument for the -f option must be begin with %";
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
                                      print_format, num_column, warning_type,
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
