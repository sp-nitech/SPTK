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
//                1996-2019  Nagoya Institute of Technology          //
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

#include <getopt.h>    // getopt_long_only
#include <algorithm>   // std::max, std::min, std::transform
#include <cmath>       // std::atan2, std::sqrt
#include <fstream>     // std::ifstream
#include <functional>  // std::divides, std::minus, std::multiplies, std::plus
#include <iomanip>     // std::setw
#include <iostream>    // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>     // std::ostringstream
#include <vector>      // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace {

enum LongOptions {
  kATAN2 = 1000,
  kAM,
  kGM,
  kHM,
  kMAX,
  kMIN,
  kEQ,
  kNE,
  kLT,
  kLE,
  kGT,
  kGE,
};

enum InputFormats { kNaive = 0, kRecursive, kInterleaved, kNumInputFormats };

const int kDefaultVectorLength(1);
const InputFormats kDefaultInputFormat(kNaive);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " vopr - perform vector operation" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       vopr [ options ] [ infile ] [ file1 ] > stdout" << std::endl;  // NOLINT
  *stream << "  options:" << std::endl;
  *stream << "       -l l   : length of vector         (   int)[" << std::setw(5) << std::right << kDefaultVectorLength << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -n n   : order of vector          (   int)[" << std::setw(5) << std::right << "l-1"                << "][ 0 <= n <=   ]" << std::endl;  // NOLINT
  *stream << "       -q q   : input format             (   int)[" << std::setw(5) << std::right << kDefaultInputFormat  << "][ 0 <= q <= 2 ]" << std::endl;  // NOLINT
  *stream << "                  0 (naive)" << std::endl;
  *stream << "                      infile: a11 a12 .. a1l  a21 a22 .. a2l  a31 a32 .. a3l  a41 a42 .. a4l" << std::endl;  // NOLINT
  *stream << "                      file1 : b11 b12 .. b1l  b21 b22 .. b2l  b31 b32 .. b3l  b41 b42 .. b4l" << std::endl;  // NOLINT
  *stream << "                  1 (recursive)" << std::endl;
  *stream << "                      infile: a11 a12 .. a1l  a21 a22 .. a2l  a31 a32 .. a3l  a41 a42 .. a4l" << std::endl;  // NOLINT
  *stream << "                      file1 : b11 b12 .. b1l" << std::endl;
  *stream << "                  2 (interleaved)" << std::endl;
  *stream << "                      infile: a11 a12 .. a1l  b11 b12 .. b2l  a21 a22 .. a2l  b21 b22 .. b2l" << std::endl;  // NOLINT
  *stream << "                      file1 : not required" << std::endl;
  *stream << "       -a     : addition                 (  bool)[FALSE][       a + b ]" << std::endl;  // NOLINT
  *stream << "       -s     : subtraction              (  bool)[FALSE][       a - b ]" << std::endl;  // NOLINT
  *stream << "       -m     : multiplication           (  bool)[FALSE][       a * b ]" << std::endl;  // NOLINT
  *stream << "       -d     : division                 (  bool)[FALSE][       a / b ]" << std::endl;  // NOLINT
  *stream << "       -ATAN2 : arctangent               (  bool)[FALSE][ atan(b / a) ]" << std::endl;  // NOLINT
  *stream << "       -AM    : arithmetic mean          (  bool)[FALSE][ (a + b) / 2 ]" << std::endl;  // NOLINT
  *stream << "       -GM    : geometric mean           (  bool)[FALSE][ sqrt(a * b) ]" << std::endl;  // NOLINT
  *stream << "       -HM    : harmonic mean            (  bool)[FALSE][ 2/(1/a+1/b) ]" << std::endl;  // NOLINT
  *stream << "       -MIN   : minimum                  (  bool)[FALSE][   min(a, b) ]" << std::endl;  // NOLINT
  *stream << "       -MAX   : maximum                  (  bool)[FALSE][   max(a, b) ]" << std::endl;  // NOLINT
  *stream << "       -EQ    : equal to                 (  bool)[FALSE][      a == b ]" << std::endl;  // NOLINT
  *stream << "       -NE    : not equal to             (  bool)[FALSE][      a != b ]" << std::endl;  // NOLINT
  *stream << "       -LT    : less than                (  bool)[FALSE][      a <  b ]" << std::endl;  // NOLINT
  *stream << "       -LE    : less than or equal to    (  bool)[FALSE][      a <= b ]" << std::endl;  // NOLINT
  *stream << "       -GT    : greater than             (  bool)[FALSE][      a >  b ]" << std::endl;  // NOLINT
  *stream << "       -GE    : greater than or equal to (  bool)[FALSE][      a >= b ]" << std::endl;  // NOLINT
  *stream << "       -h     : print this message" << std::endl;
  *stream << "  file1:" << std::endl;
  *stream << "       data sequence                     (double)" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence                     (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       data sequence after operation     (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

int main(int argc, char* argv[]) {
  int vector_length(kDefaultVectorLength);
  InputFormats input_format(kDefaultInputFormat);
  int operation_type(-1);

  const struct option long_options[] = {
      {"ATAN2", no_argument, NULL, kATAN2},
      {"AM", no_argument, NULL, kAM},
      {"GM", no_argument, NULL, kGM},
      {"HM", no_argument, NULL, kHM},
      {"MIN", no_argument, NULL, kMIN},
      {"MAX", no_argument, NULL, kMAX},
      {"EQ", no_argument, NULL, kEQ},
      {"NE", no_argument, NULL, kNE},
      {"LT", no_argument, NULL, kLT},
      {"LE", no_argument, NULL, kLE},
      {"GT", no_argument, NULL, kGT},
      {"GE", no_argument, NULL, kGE},
      {0, 0, 0, 0},
  };

  for (;;) {
    const int option_char(
        getopt_long_only(argc, argv, "l:n:q:asmdh", long_options, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &vector_length) ||
            vector_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("vopr", error_message);
          return 1;
        }
        break;
      }
      case 'n': {
        if (!sptk::ConvertStringToInteger(optarg, &vector_length) ||
            vector_length < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -n option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("vopr", error_message);
          return 1;
        }
        ++vector_length;
        break;
      }
      case 'q': {
        const int min(0);
        const int max(static_cast<int>(kNumInputFormats) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -q option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("vopr", error_message);
          return 1;
        }
        input_format = static_cast<InputFormats>(tmp);
        break;
      }
      case 'a':
      case 's':
      case 'm':
      case 'd':
      case kATAN2:
      case kAM:
      case kGM:
      case kHM:
      case kMAX:
      case kMIN:
      case kEQ:
      case kNE:
      case kLT:
      case kLE:
      case kGT:
      case kGE: {
        operation_type = option_char;
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

  if (-1 == operation_type) {
    std::ostringstream error_message;
    error_message << "No operation is specified";
    sptk::PrintErrorMessage("vopr", error_message);
    return 1;
  }

  std::ifstream infile_ifs;
  std::ifstream file1_ifs;

  switch (input_format) {
    case kNaive:
    case kRecursive: {
      const char* infile;
      const char* file1;
      const int num_input_files(argc - optind);
      if (2 == num_input_files) {
        infile = argv[argc - 2];
        file1 = argv[argc - 1];
      } else if (1 == num_input_files) {
        file1 = argv[argc - 1];
        infile = NULL;
      } else {
        std::ostringstream error_message;
        error_message << "Just two input files, file1 and infile, are required";
        sptk::PrintErrorMessage("vopr", error_message);
        return 1;
      }

      infile_ifs.open(infile, std::ios::in | std::ios::binary);
      if (infile_ifs.fail() && NULL != infile) {
        std::ostringstream error_message;
        error_message << "Cannot open file " << infile;
        sptk::PrintErrorMessage("vopr", error_message);
        return 1;
      }

      file1_ifs.open(file1, std::ios::in | std::ios::binary);
      if (file1_ifs.fail()) {
        std::ostringstream error_message;
        error_message << "Cannot open file " << file1;
        sptk::PrintErrorMessage("vopr", error_message);
        return 1;
      }
      break;
    }
    case kInterleaved: {
      const int num_input_files(argc - optind);
      if (1 < num_input_files) {
        std::ostringstream error_message;
        error_message << "Too many input files";
        sptk::PrintErrorMessage("vopr", error_message);
        return 1;
      }

      const char* infile(0 == num_input_files ? NULL : argv[optind]);
      infile_ifs.open(infile, std::ios::in | std::ios::binary);
      if (infile_ifs.fail() && NULL != infile) {
        std::ostringstream error_message;
        error_message << "Cannot open file " << infile;
        sptk::PrintErrorMessage("vopr", error_message);
        return 1;
      }
      break;
    }
    default: { return 0; }
  }

  std::istream& infile_stream(infile_ifs.fail() ? std::cin : infile_ifs);
  std::istream& file1_stream(file1_ifs);

  std::vector<double> vector_a(vector_length);
  std::vector<double> vector_b(vector_length);
  std::vector<double> result(vector_length);

  if (kRecursive == input_format &&
      !sptk::ReadStream(false, 0, 0, vector_length, &vector_b, &file1_stream,
                        NULL)) {
    return 0;
  }

  while (sptk::ReadStream(false, 0, 0, vector_length, &vector_a, &infile_stream,
                          NULL)) {
    if (kNaive == input_format &&
        !sptk::ReadStream(false, 0, 0, vector_length, &vector_b, &file1_stream,
                          NULL)) {
      break;
    }
    if (kInterleaved == input_format &&
        !sptk::ReadStream(false, 0, 0, vector_length, &vector_b, &infile_stream,
                          NULL)) {
      break;
    }

    switch (operation_type) {
      case 'a': {
        std::transform(vector_a.begin(), vector_a.end(), vector_b.begin(),
                       result.begin(), std::plus<double>());
        break;
      }
      case 's': {
        std::transform(vector_a.begin(), vector_a.end(), vector_b.begin(),
                       result.begin(), std::minus<double>());
        break;
      }
      case 'm': {
        std::transform(vector_a.begin(), vector_a.end(), vector_b.begin(),
                       result.begin(), std::multiplies<double>());
        break;
      }
      case 'd': {
        std::transform(vector_a.begin(), vector_a.end(), vector_b.begin(),
                       result.begin(), std::divides<double>());
        break;
      }
      case kATAN2: {
        std::transform(vector_a.begin(), vector_a.end(), vector_b.begin(),
                       result.begin(),
                       [](double a, double b) { return std::atan2(b, a); });
        break;
      }
      case kAM: {
        std::transform(vector_a.begin(), vector_a.end(), vector_b.begin(),
                       result.begin(),
                       [](double a, double b) { return 0.5 * (a + b); });
        break;
      }
      case kGM: {
        std::transform(vector_a.begin(), vector_a.end(), vector_b.begin(),
                       result.begin(),
                       [](double a, double b) { return std::sqrt(a * b); });
        break;
      }
      case kHM: {
        std::transform(
            vector_a.begin(), vector_a.end(), vector_b.begin(), result.begin(),
            [](double a, double b) { return 2.0 / (1.0 / a + 1.0 / b); });
        break;
      }
      case kMIN: {
        std::transform(vector_a.begin(), vector_a.end(), vector_b.begin(),
                       result.begin(),
                       [](double a, double b) { return std::min(a, b); });
        break;
      }
      case kMAX: {
        std::transform(vector_a.begin(), vector_a.end(), vector_b.begin(),
                       result.begin(),
                       [](double a, double b) { return std::max(a, b); });
        break;
      }
      case kEQ: {
        std::transform(vector_a.begin(), vector_a.end(), vector_b.begin(),
                       result.begin(),
                       [](double a, double b) { return a == b; });
        break;
      }
      case kNE: {
        std::transform(vector_a.begin(), vector_a.end(), vector_b.begin(),
                       result.begin(),
                       [](double a, double b) { return a != b; });
        break;
      }
      case kLT: {
        std::transform(vector_a.begin(), vector_a.end(), vector_b.begin(),
                       result.begin(),
                       [](double a, double b) { return a < b; });
        break;
      }
      case kLE: {
        std::transform(vector_a.begin(), vector_a.end(), vector_b.begin(),
                       result.begin(),
                       [](double a, double b) { return a <= b; });
        break;
      }
      case kGT: {
        std::transform(vector_a.begin(), vector_a.end(), vector_b.begin(),
                       result.begin(),
                       [](double a, double b) { return b < a; });
        break;
      }
      case kGE: {
        std::transform(vector_a.begin(), vector_a.end(), vector_b.begin(),
                       result.begin(),
                       [](double a, double b) { return b <= a; });
        break;
      }
      default: { break; }
    }

    if (!sptk::WriteStream(0, vector_length, result, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write data";
      sptk::PrintErrorMessage("vopr", error_message);
      return 1;
    }
  }

  return 0;
}
