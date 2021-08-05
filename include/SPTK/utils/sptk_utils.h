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
//                1996-2021  Nagoya Institute of Technology          //
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

#include <cstddef>   // std::size_t
#include <iostream>  // std::istream, std::ostream
#include <sstream>   // std::ostringstream
#include <string>    // std::string
#include <vector>    // std::vector

#include "SPTK/math/matrix.h"
#include "SPTK/math/symmetric_matrix.h"

#ifndef DISALLOW_COPY_AND_ASSIGN
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)
#endif

namespace sptk {

//! Version of SPTK.
static const char* const kVersion("4.0");
//! @f$\pi@f$
static const double kPi(3.141592653589793);
//! @f$2\pi@f$
static const double kTwoPi(6.283185307179586);
//! @f$20 / \ln(10)@f$
static const double kNeper(8.685889638065035);
//! @f$1 / \ln(2)@f$
static const double kOctave(1.442695040888963);
//! @f$\ln(2)@f$
static const double kLogTwo(0.693147180559945);
//! @f$\ln(0)@f$
static const double kLogZero(-1.0e+10);

/**
 * @param[out] data_to_read Scalar.
 * @param[out] input_stream Stream to be read.
 * @return True on success, false on failure.
 */
template <typename T>
bool ReadStream(T* data_to_read, std::istream* input_stream);

/**
 * @param[out] matrix_to_read Matrix.
 * @param[out] input_stream Stream to be read.
 * @return True on success, false on failure.
 */
bool ReadStream(sptk::Matrix* matrix_to_read, std::istream* input_stream);

/**
 * @param[out] matrix_to_read Symmetric matrix.
 * @param[out] input_stream Stream to be read.
 * @return True on success, false on failure.
 */
bool ReadStream(sptk::SymmetricMatrix* matrix_to_read,
                std::istream* input_stream);

/**
 * @param[in] zero_padding If true and @f$L'>0@f$, pad @f$L-L'@f$ elements
 *            with zeros and return true. If false, padding is not performed
 *            and return false.
 * @param[in] stream_skip Skip size.
 * @param[in] read_point Insert index in vector.
 * @param[in] read_size Target read size, @f$L@f$.
 * @param[out] sequence_to_read Vector.
 * @param[out] input_stream Stream to be read.
 * @param[out] actual_read_size Actual read size, @f$L'@f$.
 * @return True on success, false on failure.
 */
template <typename T>
bool ReadStream(bool zero_padding, int stream_skip, int read_point,
                int read_size, std::vector<T>* sequence_to_read,
                std::istream* input_stream, int* actual_read_size);

/**
 * @param[in] data_to_write Scalar.
 * @param[out] output_stream Stream to be write.
 * @return True on success, false on failure.
 */
template <typename T>
bool WriteStream(T data_to_write, std::ostream* output_stream);

/**
 * @param[in] matrix_to_write Matrix.
 * @param[out] output_stream Stream to be write.
 * @return True on success, false on failure.
 */
bool WriteStream(const sptk::Matrix& matrix_to_write,
                 std::ostream* output_stream);

/**
 * @param[in] matrix_to_write Symmetric matrix.
 * @param[out] output_stream Stream to be write.
 * @return True on success, false on failure.
 */
bool WriteStream(const sptk::SymmetricMatrix& matrix_to_write,
                 std::ostream* output_stream);

/**
 * @param[in] write_point Start index.
 * @param[in] write_size Target write size.
 * @param[in] sequence_to_write Vector.
 * @param[out] output_stream Stream to be write.
 * @param[out] actual_write_size Actual write size.
 * @return True on success, false on failure.
 */
template <typename T>
bool WriteStream(int write_point, int write_size,
                 const std::vector<T>& sequence_to_write,
                 std::ostream* output_stream, int* actual_write_size);

/**
 * @param[in] data Data.
 * @param[in] print_format Print format.
 * @param[in] buffer_size Buffer size.
 * @param[out] buffer Formatted data.
 * @return True on success, false on failure.
 */
template <typename T>
bool SnPrintf(T data, const std::string& print_format, std::size_t buffer_size,
              char* buffer);

/**
 * @param[in] input Boolean.
 * @return Converted string, "TRUE" or "FALSE".
 */
const char* ConvertBooleanToString(bool input);

/**
 * @param[in] input String.
 * @param[out] output Converted integer value.
 * @return True on success, false on failure.
 */
bool ConvertStringToInteger(const std::string& input, int* output);

/**
 * @param[in] input String.
 * @param[out] output Converted float value.
 * @return True on success, false on failure.
 */
bool ConvertStringToDouble(const std::string& input, double* output);

/**
 * @param[in] input String can be "pi", "db", "cent", "semitone", "octave",
 *            "sqrtX", "lnX", "expX", and "X", where "X" is a number.
 * @param[out] output Converted float value.
 * @return True on success, false on failure.
 */
bool ConvertSpecialStringToDouble(const std::string& input, double* output);

/**
 * @param[in] num A number.
 * @return True if given number is even, false otherwise.
 */
bool IsEven(int num);

/**
 * @param[in] num A number.
 * @param[in] min Minimum value.
 * @param[in] max Maximum value.
 * @return True if given number is in [min, max], false otherwise.
 */
bool IsInRange(int num, int min, int max);

/**
 * @param[in] num A number.
 * @param[in] min Minimum value.
 * @param[in] max Maximum value.
 * @return True if given number is in [min, max], false otherwise.
 */
bool IsInRange(double num, double min, double max);

/**
 * @param[in] num A number.
 * @return True if given number is @f$2^n@f$, where @f$n@f$ is a non-negative
 *         number, false otherwise.
 */
bool IsPowerOfTwo(int num);

/**
 * @param[in] alpha All-pass constant, @f$\alpha@f$.
 * @return True if given alpha satisfy @f$|\alpha| < 1@f$, false otherwise.
 */
bool IsValidAlpha(double alpha);

/**
 * @param[in] gamma Gamma, @f$\gamma@f$.
 * @return True if given gamma satisfy @f$|\gamma| \le 1@f$, false otherwise.
 */
bool IsValidGamma(double gamma);

/**
 * @param[in] num A number, @f$x@f$.
 * @return The smallest @f$2^n@f$ greater than or equal to @f$x@f$, where
 *         @f$n@f$ is a non-negative number.
 */
int NextPowTwo(int num);

/**
 * @param[in] num A number, @f$x@f$.
 * @return @f$\mathrm{sgn}(x)@f$.
 */
int ExtractSign(double num);

/**
 * @param[in] num A number, @f$x@f$.
 * @return @f$\max(\ln(x), -1 \times 10^{10})@f$.
 */
double FloorLog(double num);

/**
 * @param[in] num A number, @f$x@f$.
 * @return @f$\max(\log_2(x), -1 \times 10^{10})@f$.
 */
double FloorLog2(double num);

/**
 * @param[in] num A number, @f$x@f$.
 * @return @f$\max(\log_{10}(x), -1 \times 10^{10})@f$.
 */
double FloorLog10(double num);

/**
 * @param[in] log_x @f$\ln(x)@f$.
 * @param[in] log_y @f$\ln(y)@f$.
 * @return @f$\ln(x + y)@f$.
 */
double AddInLogSpace(double log_x, double log_y);

/**
 * @param[in] symbol A character represents data type.
 * @param[out] stream Stream to be written readable data type.
 */
void PrintDataType(const std::string& symbol, std::ostream* stream);

/**
 * Print error message to standard error.
 *
 * @param[in] program_name Name of SPTK command.
 * @param[in] message Error message.
 */
void PrintErrorMessage(const std::string& program_name,
                       const std::ostringstream& message);

}  // namespace sptk

#endif  // SPTK_UTILS_SPTK_UTILS_H_
