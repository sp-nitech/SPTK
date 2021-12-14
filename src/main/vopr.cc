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

#include <algorithm>   // std::max, std::min, std::transform
#include <cmath>       // std::atan2, std::sqrt
#include <fstream>     // std::ifstream
#include <functional>  // std::divides, std::minus, std::multiplies, std::plus
#include <iomanip>     // std::setw
#include <iostream>    // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>     // std::ostringstream
#include <vector>      // std::vector

#include "Getopt/getoptwin.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum LongOptions {
  kATAN = 1000,
  kQM,
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
  *stream << "       -l l  : length of vector         (   int)[" << std::setw(5) << std::right << kDefaultVectorLength << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -n n  : order of vector          (   int)[" << std::setw(5) << std::right << "l-1"                << "][ 0 <= n <=   ]" << std::endl;  // NOLINT
  *stream << "       -q q  : input format             (   int)[" << std::setw(5) << std::right << kDefaultInputFormat  << "][ 0 <= q <= 2 ]" << std::endl;  // NOLINT
  *stream << "                  0 (naive)" << std::endl;
  *stream << "                      infile: a11 a12 .. a1l  a21 a22 .. a2l  a31 a32 .. a3l  a41 a42 .. a4l" << std::endl;  // NOLINT
  *stream << "                      file1 : b11 b12 .. b1l  b21 b22 .. b2l  b31 b32 .. b3l  b41 b42 .. b4l" << std::endl;  // NOLINT
  *stream << "                  1 (recursive)" << std::endl;
  *stream << "                      infile: a11 a12 .. a1l  a21 a22 .. a2l  a31 a32 .. a3l  a41 a42 .. a4l" << std::endl;  // NOLINT
  *stream << "                      file1 : b11 b12 .. b1l" << std::endl;
  *stream << "                  2 (interleaved)" << std::endl;
  *stream << "                      infile: a11 a12 .. a1l  b11 b12 .. b2l  a21 a22 .. a2l  b21 b22 .. b2l" << std::endl;  // NOLINT
  *stream << "                      file1 : not required" << std::endl;
  *stream << "       -a    : addition                                [       a + b ]" << std::endl;  // NOLINT
  *stream << "       -s    : subtraction                             [       a - b ]" << std::endl;  // NOLINT
  *stream << "       -m    : multiplication                          [       a * b ]" << std::endl;  // NOLINT
  *stream << "       -d    : division                                [       a / b ]" << std::endl;  // NOLINT
  *stream << "       -ATAN : arctangent                              [ atan(b / a) ]" << std::endl;  // NOLINT
  *stream << "       -QM   : quadratic mean                          [ sqrt(a^2 + b^2 / 2) ]" << std::endl;  // NOLINT
  *stream << "       -AM   : arithmetic mean                         [ (a + b) / 2 ]" << std::endl;  // NOLINT
  *stream << "       -GM   : geometric mean                          [ sqrt(a * b) ]" << std::endl;  // NOLINT
  *stream << "       -HM   : harmonic mean                           [ 2 / (1 / a + 1 / b) ]" << std::endl;  // NOLINT
  *stream << "       -MIN  : minimum                                 [   min(a, b) ]" << std::endl;  // NOLINT
  *stream << "       -MAX  : maximum                                 [   max(a, b) ]" << std::endl;  // NOLINT
  *stream << "       -EQ   : equal to                                [      a == b ]" << std::endl;  // NOLINT
  *stream << "       -NE   : not equal to                            [      a != b ]" << std::endl;  // NOLINT
  *stream << "       -LT   : less than                               [      a <  b ]" << std::endl;  // NOLINT
  *stream << "       -LE   : less than or equal to                   [      a <= b ]" << std::endl;  // NOLINT
  *stream << "       -GT   : greater than                            [      a >  b ]" << std::endl;  // NOLINT
  *stream << "       -GE   : greater than or equal to                [      a >= b ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
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

/**
 * @a vopr [ @e option ] [ @e infile ] [ @e file1 ] > stdout
 *
 * - @b -l @e int
 *   - length of vector @f$(1 \le L)@f$
 * - @b -n @e int
 *   - order of vector @f$(0 \le L - 1)@f$
 * - @b -q @e int
 *   - input format
 *     \arg @c 0 naive
 *     \arg @c 1 recursive
 *     \arg @c 2 interleaved
 * - @b -a
 *   - addition
 * - @b -s
 *   - subtraction
 * - @b -m
 *   - multiplication
 * - @b -d
 *   - division
 * - @b -ATAN
 *   - arctangent
 * - @b -QM
 *   - quadratic mean
 * - @b -AM
 *   - arithmetric mean
 * - @b -GM
 *   - geometric mean
 * - @b -HM
 *   - harmonic mean
 * - @b -MIN
 *   - minimum
 * - @b -MAX
 *   - maximum
 * - @b -EQ
 *   - equal to
 * - @b -NE
 *   - not equal to
 * - @b -LT
 *   - less than
 * - @b -LE
 *   - less than or equal to
 * - @b -GT
 *   - greater than
 * - @b -GE
 *   - greater than or equal to
 * - @b infile @e str
 *   - double-type data sequence
 * - @b file1 @e str
 *   - double-type data sequence
 * - @b stdout
 *   - double-type data sequence after operation
 *
 * This command performs vector operations between two sequences.
 *
 * The inputs are
 * @f[
 *   \begin{array}{ccc}
 *     \boldsymbol{a}_{0}, & \boldsymbol{a}_{1}, & \ldots,
 *   \end{array}
 * @f]
 * and
 * @f[
 *   \begin{array}{ccc}
 *     \boldsymbol{b}_{0}, & \boldsymbol{b}_{1}, & \ldots,
 *   \end{array}
 * @f]
 * where @f$\boldsymbol{a}_t@f$ and @f$\boldsymbol{b}_t@f$ are
 * @f$L@f$-length vectors.
 * The output is
 * @f[
 *   \begin{array}{ccc}
 *     \boldsymbol{y}_{0}, & \boldsymbol{y}_{1}, & \ldots,
 *   \end{array}
 * @f]
 * where
 * @f[
 *   \boldsymbol{y}_{t} = f(\boldsymbol{a}_{t}, \boldsymbol{b}_{t})
 * @f]
 * and @f$f(\cdot)@f$ is a function which returns an
 * @f$L@f$-length vector.
 *
 * There are three kinds of input formats.
 * - Case @c -q0
 * @f[
 *   \left\{ \begin{array}{ll}
 *     \boldsymbol{a}_0, \boldsymbol{a}_1, \ldots, & (\mbox{infile}) \\
 *     \boldsymbol{b}_0, \boldsymbol{b}_1, \ldots, & (\mbox{file1})
 *   \end{array} \right.
 * @f]
 * - Case @c -q1
 * @f[
 *   \left\{ \begin{array}{ll}
 *     \boldsymbol{a}_0, \boldsymbol{a}_1, \ldots, & (\mbox{infile}) \\
 *     \boldsymbol{b}, & (\mbox{file1})
 *   \end{array} \right.
 * @f]
 * where @f$\boldsymbol{b}_t=\boldsymbol{b}@f$ for any @f$t@f$.
 * - Case @c -q2
 * @f[
 *   \left\{ \begin{array}{ll}
 *     \boldsymbol{a}_0, \boldsymbol{b}_0,
 *     \boldsymbol{a}_1, \boldsymbol{b}_1, \ldots, & (\mbox{infile}) \\
 *   \end{array} \right.
 * @f]
 *
 * @code{.sh}
 *   echo 1 2 3 4 5 6 | x2x +ad > data.a
 *   echo 3 2 1 0 5 6 | x2x +ad > data.b
 *
 *   vopr -GT data.a data.b | x2x +da
 *   # 0, 0, 1, 1, 0, 0
 *   vopr -q 0 -l 3 -s data.a data.b | x2x +da
 *   # -2, 0, 2, 4, 0, 0
 *   vopr -q 1 -l 3 -s data.a data.b | x2x +da
 *   # -2, 0, 2, 1, 3, 5
 *   vopr -q 2 -l 3 -s data.a | x2x +da
 *   # -3, -3, -3
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int vector_length(kDefaultVectorLength);
  InputFormats input_format(kDefaultInputFormat);
  int operation_type(-1);

  const struct option long_options[] = {
      {"ATAN", no_argument, NULL, kATAN}, {"QM", no_argument, NULL, kQM},
      {"AM", no_argument, NULL, kAM},     {"GM", no_argument, NULL, kGM},
      {"HM", no_argument, NULL, kHM},     {"MIN", no_argument, NULL, kMIN},
      {"MAX", no_argument, NULL, kMAX},   {"EQ", no_argument, NULL, kEQ},
      {"NE", no_argument, NULL, kNE},     {"LT", no_argument, NULL, kLT},
      {"LE", no_argument, NULL, kLE},     {"GT", no_argument, NULL, kGT},
      {"GE", no_argument, NULL, kGE},     {0, 0, 0, 0},
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
      case kATAN:
      case kQM:
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
    default: {
      return 0;
    }
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
      case kATAN: {
        std::transform(vector_a.begin(), vector_a.end(), vector_b.begin(),
                       result.begin(),
                       [](double a, double b) { return std::atan2(b, a); });
        break;
      }
      case kQM: {
        std::transform(vector_a.begin(), vector_a.end(), vector_b.begin(),
                       result.begin(), [](double a, double b) {
                         return std::sqrt(0.5 * (a * a + b * b));
                       });
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
      default: {
        break;
      }
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
