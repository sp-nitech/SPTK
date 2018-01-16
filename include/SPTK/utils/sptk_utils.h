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

#ifndef SPTK_UTILS_SPTK_UTILS_H_
#define SPTK_UTILS_SPTK_UTILS_H_

#include <iostream>  // std::istream, std::ostream
#include <sstream>   // std::ostringstream
#include <string>    // std::string
#include <vector>    // std::vector

#ifndef DISALLOW_COPY_AND_ASSIGN
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)
#endif

namespace sptk {

static const char* const kVersion("4.0");
static const double kPi(3.141592653589793);
static const double kTwoPi(6.283185307179586);
static const double kNp(8.685889638065035);  // 1 Np = 20 / ln(10) dB
static const double kLogTwo(0.693147180559945);
static const double kLogZero(-1.0e+10);

template <typename T>
bool ReadStream(T* data_to_read, std::istream* input_stream);
template <typename T>
bool ReadStream(bool zero_padding, int stream_skip, int read_point,
                int read_size, std::vector<T>* sequence_to_read,
                std::istream* input_stream, int* actual_read_size);
template <typename T>
bool WriteStream(T data_to_write, std::ostream* output_stream);
template <typename T>
bool WriteStream(int write_point, int write_size,
                 const std::vector<T>& sequence_to_write,
                 std::ostream* output_stream, int* actual_write_size);
template <typename T>
bool SnPrintf(T data, const std::string& print_format, size_t buffer_size,
              char* buffer);
const char* ConvertBooleanToString(bool input);
bool ConvertStringToInteger(const std::string& input, int* output);
bool ConvertStringToDouble(const std::string& input, double* output);
bool IsInRange(int num, int min, int max);
bool IsPowerOfTwo(int num);
int ExtractSign(double x);
double FloorLog(double x);
double AddInLogSpace(double log_x, double log_y);
void PrintDataType(const std::string& symbol, std::ostream* stream);
void PrintErrorMessage(const std::string& program_name,
                       const std::ostringstream& message);

}  // namespace sptk

#endif  // SPTK_UTILS_SPTK_UTILS_H_
