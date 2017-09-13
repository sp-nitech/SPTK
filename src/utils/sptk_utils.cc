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

#include "SPTK/utils/sptk_utils.h"

#include <algorithm>  // std::fill_n
#include <cerrno>     // errno, ERANGE
#include <cmath>      // std::ceil, std::exp, std::log
#include <cstdint>    // int8_t, etc.
#include <cstdlib>    // std::size_t, std::strtod, std::strtol

#include "SPTK/utils/int24_t.h"
#include "SPTK/utils/uint24_t.h"

namespace {

// 34 is a reasonable number near log(1e-15)
static const double kThresholdOfInformationLossInLogSpace(-34.0);

}  // namespace

namespace sptk {

template <typename T>
bool ReadStream(T* read_data, std::istream* input_stream) {
  if (NULL == read_data || NULL == input_stream || input_stream->eof()) {
    return false;
  }

  const int type_byte(sizeof(*read_data));
  input_stream->read(reinterpret_cast<char*>(read_data), type_byte);

  return (type_byte == input_stream->gcount()) ? !input_stream->fail() : false;
}

template <typename T>
bool ReadStream(bool zero_padding, int stream_skip, int read_point,
                int read_size, std::vector<T>* sequence_to_read,
                std::istream* input_stream) {
  if (stream_skip < 0 || read_point < 0 || read_size <= 0 ||
      NULL == sequence_to_read || NULL == input_stream || input_stream->eof()) {
    return false;
  }

  const int type_byte(sizeof((*sequence_to_read)[0]));

  if (0 < stream_skip) {
    input_stream->ignore(type_byte * stream_skip);
    if (input_stream->eof()) return false;
  }

  const int end(read_point + read_size);
  if (sequence_to_read->size() < static_cast<std::size_t>(end)) {
    sequence_to_read->resize(end);
  }

  const int num_read_bytes(type_byte * read_size);
  input_stream->read(
      reinterpret_cast<char*>(&((*sequence_to_read)[0]) + read_point),
      num_read_bytes);

  const int gcount(input_stream->gcount());
  if (num_read_bytes == gcount) {
    return !input_stream->fail();
  } else if (zero_padding && 0 < gcount) {
    // Use std::ceil to zero incomplete data
    // as gcount may not be a multiple of sizeof(double).
    const int num_zeros(
        std::ceil(static_cast<double>(num_read_bytes - gcount) / type_byte));
    if (num_zeros < 0) {
      return false;  // Something wrong!
    }

    std::fill_n(sequence_to_read->begin() + end - num_zeros, num_zeros, 0.0);

    return !input_stream->bad();
  }

  return false;
}

template <typename T>
bool WriteStream(T data_to_write, std::ostream* output_stream) {
  if (NULL == output_stream) {
    return false;
  }

  output_stream->write(reinterpret_cast<const char*>(&data_to_write),
                       sizeof(data_to_write));

  return !output_stream->fail();
}

template <typename T>
bool WriteStream(int write_point, int write_size,
                 const std::vector<T>& sequence_to_write,
                 std::ostream* output_stream) {
  if (write_point < 0 || write_size <= 0 || NULL == output_stream) {
    return false;
  }

  const int end(write_point + write_size);
  if (sequence_to_write.size() < static_cast<std::size_t>(end)) {
    return false;
  }

  output_stream->write(
      reinterpret_cast<const char*>(&(sequence_to_write[0]) + write_point),
      sizeof(sequence_to_write[0]) * write_size);

  return !output_stream->fail();
}

const char* ConvertBooleanToString(bool input) {
  return input ? "TRUE" : "FALSE";
}

bool ConvertStringToInteger(const std::string& input, int* output) {
  if (input.empty() || NULL == output) {
    return false;
  }

  char* end;
  int converted_value(static_cast<int>(std::strtol(input.c_str(), &end, 10)));

  if (0 == input.compare(end) || '\0' != *end || ERANGE == errno) {
    return false;
  }

  *output = converted_value;
  return true;
}

bool ConvertStringToDouble(const std::string& input, double* output) {
  if (input.empty() || NULL == output) {
    return false;
  }

  char* end;
  double converted_value(std::strtod(input.c_str(), &end));

  if (0 == input.compare(end) || '\0' != *end || ERANGE == errno) {
    return false;
  }

  *output = converted_value;
  return true;
}

bool IsInRange(int num, int min, int max) {
  return (min <= num && num <= max);
}

// check whether the given number is a power of two, 2^p where p is a
// non-negative integer.
bool IsPowerOfTwo(int num) {
  return !((num < 1) || (num & (num - 1)));
}

int ExtractSign(double x) {
  if (0.0 < x) return 1;
  if (x < 0.0) return -1;
  return 0;
}

double FloorLog(double x) {
  return (x <= 0.0) ? sptk::kLogZero : std::log(x);
}

// compute log(x + y) given log(x) and log(y).
double AddInLogSpace(double log_x, double log_y) {
  if (log_x == log_y) return log_x + sptk::kLogTwo;

  const double smaller((log_x < log_y) ? log_x : log_y);
  const double greater((log_x < log_y) ? log_y : log_x);
  const double diff(smaller - greater);
  if (diff < kThresholdOfInformationLossInLogSpace) return greater;
  return greater + std::log(std::exp(diff) + 1.0);
}

void PrintErrorMessage(const std::string& program_name,
                       const std::ostringstream& message) {
  std::ostringstream stream;
  stream << program_name << ": " << message.str() << "!" << std::endl;
  std::cerr << stream.str();
}

// clang-format off
template bool ReadStream<double>(double*, std::istream*);
template bool ReadStream<int8_t>(bool, int, int, int, std::vector<int8_t>*, std::istream*);                  // NOLINT
template bool ReadStream<int16_t>(bool, int, int, int, std::vector<int16_t>*, std::istream*);                // NOLINT
template bool ReadStream<sptk::int24_t>(bool, int, int, int, std::vector<sptk::int24_t>*, std::istream*);    // NOLINT
template bool ReadStream<int32_t>(bool, int, int, int, std::vector<int32_t>*, std::istream*);                // NOLINT
template bool ReadStream<int64_t>(bool, int, int, int, std::vector<int64_t>*, std::istream*);                // NOLINT
template bool ReadStream<uint8_t>(bool, int, int, int, std::vector<uint8_t>*, std::istream*);                // NOLINT
template bool ReadStream<uint16_t>(bool, int, int, int, std::vector<uint16_t>*, std::istream*);              // NOLINT
template bool ReadStream<sptk::uint24_t>(bool, int, int, int, std::vector<sptk::uint24_t>*, std::istream*);  // NOLINT
template bool ReadStream<uint32_t>(bool, int, int, int, std::vector<uint32_t>*, std::istream*);              // NOLINT
template bool ReadStream<uint64_t>(bool, int, int, int, std::vector<uint64_t>*, std::istream*);              // NOLINT
template bool ReadStream<float>(bool, int, int, int, std::vector<float>*, std::istream*);                    // NOLINT
template bool ReadStream<double>(bool, int, int, int, std::vector<double>*, std::istream*);                  // NOLINT
template bool ReadStream<long double>(bool, int, int, int, std::vector<long double>*, std::istream*);        // NOLINT
template bool WriteStream<double>(double, std::ostream*);
template bool WriteStream<int8_t>(int, int, const std::vector<int8_t>&, std::ostream*);                      // NOLINT
template bool WriteStream<int16_t>(int, int, const std::vector<int16_t>&, std::ostream*);                    // NOLINT
template bool WriteStream<sptk::int24_t>(int, int, const std::vector<sptk::int24_t>&, std::ostream*);        // NOLINT
template bool WriteStream<int32_t>(int, int, const std::vector<int32_t>&, std::ostream*);                    // NOLINT
template bool WriteStream<int64_t>(int, int, const std::vector<int64_t>&, std::ostream*);                    // NOLINT
template bool WriteStream<uint8_t>(int, int, const std::vector<uint8_t>&, std::ostream*);                    // NOLINT
template bool WriteStream<uint16_t>(int, int, const std::vector<uint16_t>&, std::ostream*);                  // NOLINT
template bool WriteStream<sptk::uint24_t>(int, int, const std::vector<sptk::uint24_t>&, std::ostream*);      // NOLINT
template bool WriteStream<uint32_t>(int, int, const std::vector<uint32_t>&, std::ostream*);                  // NOLINT
template bool WriteStream<uint64_t>(int, int, const std::vector<uint64_t>&, std::ostream*);                  // NOLINT
template bool WriteStream<float>(int, int, const std::vector<float>&, std::ostream*);                        // NOLINT
template bool WriteStream<double>(int, int, const std::vector<double>&, std::ostream*);                      // NOLINT
template bool WriteStream<long double>(int, int, const std::vector<long double>&, std::ostream*);            // NOLINT
// clang-format on

}  // namespace sptk
