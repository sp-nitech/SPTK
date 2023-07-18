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

#include <fstream>   // std::ifstream
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream

#include "GETOPT/ya_getopt.h"
#include "SPTK/math/scalar_operation.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum LongOptions {
  kABS = 1000,
  kINV,
  kSQR,
  kSQRT,
  kLN,
  kLOG2,
  kLOG10,
  kLOGX,
  kEXP,
  kPOW2,
  kPOW10,
  kPOWX,
  kFLOOR,
  kCEIL,
  kROUND,
  kROUNDUP,
  kROUNDDOWN,
  kUNIT,
  kRAMP,
  kSIGN,
  kSIN,
  kCOS,
  kTAN,
  kATAN,
  kTANH,
  kATANH,
  kMagic,
  kMAGIC,
};

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " sopr - perform scalar operations" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       sopr [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -a a         : addition             (double)[  N/A][      x + a ]" << std::endl;  // NOLINT
  *stream << "       -s s         : subtraction          (double)[  N/A][      x - s ]" << std::endl;  // NOLINT
  *stream << "       -m m         : multiplication       (double)[  N/A][      x * m ]" << std::endl;  // NOLINT
  *stream << "       -d d         : division             (double)[  N/A][      x / d ]" << std::endl;  // NOLINT
  *stream << "       -r r         : modulo               (double)[  N/A][      x % r ]" << std::endl;  // NOLINT
  *stream << "       -p p         : power                (double)[  N/A][      x ^ p ]" << std::endl;  // NOLINT
  *stream << "       -l l         : lower bounding       (double)[  N/A][  max(x, l) ]" << std::endl;  // NOLINT
  *stream << "       -u u         : upper bounding       (double)[  N/A][  min(x, u) ]" << std::endl;  // NOLINT
  *stream << "       -ABS         : absolute                            [        |x| ]" << std::endl;  // NOLINT
  *stream << "       -INV         : inverse                             [      1 / x ]" << std::endl;  // NOLINT
  *stream << "       -SQR         : square                              [      x ^ 2 ]" << std::endl;  // NOLINT
  *stream << "       -SQRT        : square root                         [    x ^ 0.5 ]" << std::endl;  // NOLINT
  *stream << "       -LN          : natural logarithm                   [      ln(x) ]" << std::endl;  // NOLINT
  *stream << "       -LOG2        : base 2 logarithm                    [    log2(x) ]" << std::endl;  // NOLINT
  *stream << "       -LOG10       : base 10 logarithm                   [   log10(x) ]" << std::endl;  // NOLINT
  *stream << "       -LOGX X      : base X logarithm     (double)[  N/A][    logX(x) ]" << std::endl;  // NOLINT
  *stream << "       -EXP         : exponential                         [      e ^ x ]" << std::endl;  // NOLINT
  *stream << "       -POW2        : power of 2                          [      2 ^ x ]" << std::endl;  // NOLINT
  *stream << "       -POW10       : power of 10                         [     10 ^ x ]" << std::endl;  // NOLINT
  *stream << "       -POWX X      : power of X           (double)[  N/A][      X ^ x ]" << std::endl;  // NOLINT
  *stream << "       -FLOOR       : flooring                            [   floor(x) ]" << std::endl;  // NOLINT
  *stream << "       -CEIL        : ceiling                             [    ceil(x) ]" << std::endl;  // NOLINT
  *stream << "       -ROUND       : rounding                            [   round(x) ]" << std::endl;  // NOLINT
  *stream << "       -ROUNDUP     : rounding up                         [ roundup(x) ]" << std::endl;  // NOLINT
  *stream << "       -ROUNDDOWN   : rounding down                       [     int(x) ]" << std::endl;  // NOLINT
  *stream << "       -UNIT        : unit step                           [       u(x) ]" << std::endl;  // NOLINT
  *stream << "       -RAMP        : rectifier                           [   x * u(x) ]" << std::endl;  // NOLINT
  *stream << "       -SIGN        : sign                                [     sgn(x) ]" << std::endl;  // NOLINT
  *stream << "       -SIN         : sine                                [     sin(x) ]" << std::endl;  // NOLINT
  *stream << "       -COS         : cosine                              [     cos(x) ]" << std::endl;  // NOLINT
  *stream << "       -TAN         : tangent                             [     tan(x) ]" << std::endl;  // NOLINT
  *stream << "       -ATAN        : arctangent                          [    atan(x) ]" << std::endl;  // NOLINT
  *stream << "       -TANH        : hyperbolic tangent                  [    tanh(x) ]" << std::endl;  // NOLINT
  *stream << "       -ATANH       : hyperbolic arctangent               [   atanh(x) ]" << std::endl;  // NOLINT
  *stream << "       -magic magic : remove magic number  (double)[  N/A][            ]" << std::endl;  // NOLINT
  *stream << "       -MAGIC MAGIC : replace magic number (double)[  N/A][      MAGIC ]" << std::endl;  // NOLINT
  *stream << "       -h           : print this message" << std::endl;
  *stream << "" << std::endl;
  *stream << "       the following strings can be used as the argument of -a, -s, -m," << std::endl;  // NOLINT
  *stream << "       -d, -p, -l, -u, -magic, or -MAGIC option:" << std::endl;
  *stream << "" << std::endl;
  *stream << "           pi       : 3.14159265..." << std::endl;
  *stream << "           dB       :   20 / ln(10)" << std::endl;
  *stream << "           cent     : 1200 / ln(2)" << std::endl;
  *stream << "           semitone :   12 / ln(2)" << std::endl;
  *stream << "           octave   :    1 / ln(2)" << std::endl;
  *stream << "           sqrtX    :      sqrt(X)  [ 0.0 <= X <=   ]" << std::endl;  // NOLINT
  *stream << "           lnX      :        ln(X)  [ 0.0 <  X <=   ]" << std::endl;  // NOLINT
  *stream << "           expX     :       exp(X)  [     <= X <=   ]" << std::endl;  // NOLINT
  *stream << "" << std::endl;
  *stream << "       they are case-insensitive" << std::endl;
  *stream << "" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence                       (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       data sequence after operations      (double)" << std::endl;
  *stream << "  notice:" << std::endl;
  *stream << "       if -MAGIC is given before -magic is given, return error" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

void PrintTypeErrorMessage(const char* option) {
  std::ostringstream error_message;
  error_message << "The argument for the -" << option
                << " option must be numeric";
  sptk::PrintErrorMessage("sopr", error_message);
}

void PrintGeneralErrorMessage(const char* option) {
  std::ostringstream error_message;
  error_message << "Failed to add operation by -" << option << " option";
  sptk::PrintErrorMessage("sopr", error_message);
}

}  // namespace

/**
 * @a sopr [ @e option ] [ @e infile ]
 *
 * - @b -a @e double
 *   - addition
 * - @b -s @e double
 *   - subtraction
 * - @b -m @e double
 *   - multiplication
 * - @b -d @e double
 *   - division
 * - @b -r @e double
 *   - modulo
 * - @b -p @e double
 *   - power
 * - @b -l @e double
 *   - lower bounding
 * - @b -u @e double
 *   - upper bounding
 * - @b -ABS
 *   - absolute
 * - @b -INV
 *   - inverse
 * - @b -SQR
 *   - square
 * - @b -SQRT
 *   - square root
 * - @b -LN
 *   - natural logarithm
 * - @b -LOG2
 *   - base 2 logarithm
 * - @b -LOG10
 *   - base 10 logarithm
 * - @b -LOGX @e double
 *   - base @f$X@f$ logarithm
 * - @b -EXP
 *   - exponential
 * - @b -POW2
 *   - power of 2
 * - @b -POW10
 *   - power of 10
 * - @b -POWX @e double
 *   - power of @f$X@f$
 * - @b -FLOOR
 *   - flooring
 * - @b -CEIL
 *   - ceiling
 * - @b -ROUND
 *   - roudning
 * - @b -ROUNDUP
 *   - roudning up
 * - @b -ROUNDDOWN
 *   - roudning down
 * - @b -UNIT
 *   - unit step
 * - @b -RAMP
 *   - rectifier
 * - @b -SIGN
 *   - sign
 * - @b -SIN
 *   - sine
 * - @b -COS
 *   - cosine
 * - @b -TAN
 *   - tangent
 * - @b -ATAN
 *   - arctangent
 * - @b -TANH
 *   - hyperbolic tangent
 * - @b -ATANH
 *   - hyperbolic arctangent
 * - @b -magic @e double
 *   - remove magic number
 * - @b -MAGIC @e double
 *   - replace magic number
 * - @b infile @e str
 *   - double-type data sequence
 * - @b stdout
 *   - double-type data sequence after operations
 *
 * @code{.sh}
 *   # 0, 1, 2, 3
 *   ramp -l 4 | sopr -m 2 -a 1 | x2x +da
 *   # 1, 3, 5, 7
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  sptk::ScalarOperation scalar_operation;

  const struct option long_options[] = {
      {"ABS", no_argument, NULL, kABS},
      {"INV", no_argument, NULL, kINV},
      {"SQR", no_argument, NULL, kSQR},
      {"SQRT", no_argument, NULL, kSQRT},
      {"LN", no_argument, NULL, kLN},
      {"LOG2", no_argument, NULL, kLOG2},
      {"LOG10", no_argument, NULL, kLOG10},
      {"LOGX", required_argument, NULL, kLOGX},
      {"EXP", no_argument, NULL, kEXP},
      {"POW2", no_argument, NULL, kPOW2},
      {"POW10", no_argument, NULL, kPOW10},
      {"POWX", required_argument, NULL, kPOWX},
      {"FLOOR", no_argument, NULL, kFLOOR},
      {"CEIL", no_argument, NULL, kCEIL},
      {"ROUND", no_argument, NULL, kROUND},
      {"ROUNDUP", no_argument, NULL, kROUNDUP},
      {"ROUNDDOWN", no_argument, NULL, kROUNDDOWN},
      {"UNIT", no_argument, NULL, kUNIT},
      {"RAMP", no_argument, NULL, kRAMP},
      {"SIGN", no_argument, NULL, kSIGN},
      {"SIN", no_argument, NULL, kSIN},
      {"COS", no_argument, NULL, kCOS},
      {"TAN", no_argument, NULL, kTAN},
      {"ATAN", no_argument, NULL, kATAN},
      {"TANH", no_argument, NULL, kTANH},
      {"ATANH", no_argument, NULL, kATANH},
      {"magic", required_argument, NULL, kMagic},
      {"MAGIC", required_argument, NULL, kMAGIC},
      {0, 0, 0, 0},
  };

  for (;;) {
    const int option_char(
        getopt_long_only(argc, argv, "a:s:m:d:r:p:l:u:h", long_options, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'a': {
        double addend;
        if (!sptk::ConvertSpecialStringToDouble(optarg, &addend) &&
            !sptk::ConvertStringToDouble(optarg, &addend)) {
          PrintTypeErrorMessage("a");
          return 1;
        }
        if (!scalar_operation.AddAdditionOperation(addend)) {
          PrintGeneralErrorMessage("a");
          return 1;
        }
        break;
      }
      case 's': {
        double subtrahend;
        if (!sptk::ConvertSpecialStringToDouble(optarg, &subtrahend) &&
            !sptk::ConvertStringToDouble(optarg, &subtrahend)) {
          PrintTypeErrorMessage("s");
          return 1;
        }
        if (!scalar_operation.AddSubtractionOperation(subtrahend)) {
          PrintGeneralErrorMessage("s");
          return 1;
        }
        break;
      }
      case 'm': {
        double multiplier;
        if (!sptk::ConvertSpecialStringToDouble(optarg, &multiplier) &&
            !sptk::ConvertStringToDouble(optarg, &multiplier)) {
          PrintTypeErrorMessage("m");
          return 1;
        }
        if (!scalar_operation.AddMultiplicationOperation(multiplier)) {
          PrintGeneralErrorMessage("m");
          return 1;
        }
        break;
      }
      case 'd': {
        double divisor;
        if (!sptk::ConvertSpecialStringToDouble(optarg, &divisor) &&
            !sptk::ConvertStringToDouble(optarg, &divisor)) {
          PrintTypeErrorMessage("d");
          return 1;
        }
        if (!scalar_operation.AddDivisionOperation(divisor)) {
          PrintGeneralErrorMessage("d");
          return 1;
        }
        break;
      }
      case 'r': {
        double divisor;
        if (!sptk::ConvertSpecialStringToDouble(optarg, &divisor) &&
            !sptk::ConvertStringToDouble(optarg, &divisor)) {
          PrintTypeErrorMessage("r");
          return 1;
        }
        if (!scalar_operation.AddModuloOperation(divisor)) {
          PrintGeneralErrorMessage("r");
          return 1;
        }
        break;
      }
      case 'p': {
        double exponent;
        if (!sptk::ConvertSpecialStringToDouble(optarg, &exponent) &&
            !sptk::ConvertStringToDouble(optarg, &exponent)) {
          PrintTypeErrorMessage("p");
          return 1;
        }
        if (!scalar_operation.AddPowerOperation(exponent)) {
          PrintGeneralErrorMessage("p");
          return 1;
        }
        break;
      }
      case 'l': {
        double lower_bound;
        if (!sptk::ConvertSpecialStringToDouble(optarg, &lower_bound) &&
            !sptk::ConvertStringToDouble(optarg, &lower_bound)) {
          PrintTypeErrorMessage("l");
          return 1;
        }
        if (!scalar_operation.AddLowerBoundingOperation(lower_bound)) {
          PrintGeneralErrorMessage("l");
          return 1;
        }
        break;
      }
      case 'u': {
        double upper_bound;
        if (!sptk::ConvertSpecialStringToDouble(optarg, &upper_bound) &&
            !sptk::ConvertStringToDouble(optarg, &upper_bound)) {
          PrintTypeErrorMessage("u");
          return 1;
        }
        if (!scalar_operation.AddUpperBoundingOperation(upper_bound)) {
          PrintGeneralErrorMessage("u");
          return 1;
        }
        break;
      }
      case kABS: {
        if (!scalar_operation.AddAbsoluteOperation()) {
          PrintGeneralErrorMessage("ABS");
          return 1;
        }
        break;
      }
      case kINV: {
        if (!scalar_operation.AddReciprocalOperation()) {
          PrintGeneralErrorMessage("INV");
          return 1;
        }
        break;
      }
      case kSQR: {
        if (!scalar_operation.AddSquareOperation()) {
          PrintGeneralErrorMessage("SQR");
          return 1;
        }
        break;
      }
      case kSQRT: {
        if (!scalar_operation.AddSquareRootOperation()) {
          PrintGeneralErrorMessage("SQRT");
          return 1;
        }
        break;
      }
      case kLN: {
        if (!scalar_operation.AddNaturalLogarithmOperation()) {
          PrintGeneralErrorMessage("LN");
          return 1;
        }
        break;
      }
      case kLOG2: {
        if (!scalar_operation.AddLogarithmOperation(2.0)) {
          PrintGeneralErrorMessage("LOG2");
          return 1;
        }
        break;
      }
      case kLOG10: {
        if (!scalar_operation.AddLogarithmOperation(10.0)) {
          PrintGeneralErrorMessage("LOG10");
          return 1;
        }
        break;
      }
      case kLOGX: {
        double base;
        if (!sptk::ConvertStringToDouble(optarg, &base)) {
          PrintTypeErrorMessage("LOGX");
          return 1;
        }
        if (!scalar_operation.AddLogarithmOperation(base)) {
          PrintGeneralErrorMessage("LOGX");
          return 1;
        }
        break;
      }
      case kEXP: {
        if (!scalar_operation.AddNaturalExponentialOperation()) {
          PrintGeneralErrorMessage("EXP");
          return 1;
        }
        break;
      }
      case kPOW2: {
        if (!scalar_operation.AddExponentialOperation(2.0)) {
          PrintGeneralErrorMessage("POW2");
          return 1;
        }
        break;
      }
      case kPOW10: {
        if (!scalar_operation.AddExponentialOperation(10.0)) {
          PrintGeneralErrorMessage("POW10");
          return 1;
        }
        break;
      }
      case kPOWX: {
        double base;
        if (!sptk::ConvertStringToDouble(optarg, &base)) {
          PrintTypeErrorMessage("POWX");
          return 1;
        }
        if (!scalar_operation.AddExponentialOperation(base)) {
          PrintGeneralErrorMessage("POWX");
          return 1;
        }
        break;
      }
      case kFLOOR: {
        if (!scalar_operation.AddFlooringOperation()) {
          PrintGeneralErrorMessage("FLOOR");
          return 1;
        }
        break;
      }
      case kCEIL: {
        if (!scalar_operation.AddCeilingOperation()) {
          PrintGeneralErrorMessage("CEIL");
          return 1;
        }
        break;
      }
      case kROUND: {
        if (!scalar_operation.AddRoundingOperation()) {
          PrintGeneralErrorMessage("ROUND");
          return 1;
        }
        break;
      }
      case kROUNDUP: {
        if (!scalar_operation.AddRoundingUpOperation()) {
          PrintGeneralErrorMessage("ROUNDUP");
          return 1;
        }
        break;
      }
      case kROUNDDOWN: {
        if (!scalar_operation.AddRoundingDownOperation()) {
          PrintGeneralErrorMessage("ROUNDDOWN");
          return 1;
        }
        break;
      }
      case kUNIT: {
        if (!scalar_operation.AddUnitStepOperation()) {
          PrintGeneralErrorMessage("UNIT");
          return 1;
        }
        break;
      }
      case kRAMP: {
        if (!scalar_operation.AddLowerBoundingOperation(0.0)) {
          PrintGeneralErrorMessage("RAMP");
          return 1;
        }
        break;
      }
      case kSIGN: {
        if (!scalar_operation.AddSignOperation()) {
          PrintGeneralErrorMessage("SIGN");
          return 1;
        }
        break;
      }
      case kSIN: {
        if (!scalar_operation.AddSineOperation()) {
          PrintGeneralErrorMessage("SIN");
          return 1;
        }
        break;
      }
      case kCOS: {
        if (!scalar_operation.AddCosineOperation()) {
          PrintGeneralErrorMessage("COS");
          return 1;
        }
        break;
      }
      case kTAN: {
        if (!scalar_operation.AddTangentOperation()) {
          PrintGeneralErrorMessage("TAN");
          return 1;
        }
        break;
      }
      case kATAN: {
        if (!scalar_operation.AddArctangentOperation()) {
          PrintGeneralErrorMessage("ATAN");
          return 1;
        }
        break;
      }
      case kTANH: {
        if (!scalar_operation.AddHyperbolicTangentOperation()) {
          PrintGeneralErrorMessage("TANH");
          return 1;
        }
        break;
      }
      case kATANH: {
        if (!scalar_operation.AddHyperbolicArctangentOperation()) {
          PrintGeneralErrorMessage("ATANH");
          return 1;
        }
        break;
      }
      case kMagic: {
        double magic_number;
        if (!sptk::ConvertSpecialStringToDouble(optarg, &magic_number) &&
            !sptk::ConvertStringToDouble(optarg, &magic_number)) {
          PrintTypeErrorMessage("magic");
          return 1;
        }
        if (!scalar_operation.AddMagicNumberRemover(magic_number)) {
          std::ostringstream error_message;
          error_message << "Cannot specify -magic option multiple times";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        break;
      }
      case kMAGIC: {
        double replacement_number;
        if (!sptk::ConvertSpecialStringToDouble(optarg, &replacement_number) &&
            !sptk::ConvertStringToDouble(optarg, &replacement_number)) {
          PrintTypeErrorMessage("MAGIC");
          return 1;
        }
        if (!scalar_operation.AddMagicNumberReplacer(replacement_number)) {
          std::ostringstream error_message;
          error_message << "Cannot find -magic option before -MAGIC option";
          sptk::PrintErrorMessage("sopr", error_message);
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

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("sopr", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  if (!sptk::SetBinaryMode()) {
    std::ostringstream error_message;
    error_message << "Cannot set translation mode";
    sptk::PrintErrorMessage("sopr", error_message);
    return 1;
  }

  std::ifstream ifs;
  if (NULL != input_file) {
    ifs.open(input_file, std::ios::in | std::ios::binary);
    if (ifs.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << input_file;
      sptk::PrintErrorMessage("sopr", error_message);
      return 1;
    }
  }
  std::istream& input_stream(ifs.is_open() ? ifs : std::cin);

  double number;
  bool is_magic_number;

  while (sptk::ReadStream(&number, &input_stream)) {
    if (!scalar_operation.Run(&number, &is_magic_number)) {
      std::ostringstream error_message;
      error_message << "Failed to perform scalar operation";
      sptk::PrintErrorMessage("sopr", error_message);
      return 1;
    }
    if (!is_magic_number && !sptk::WriteStream(number, &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write data";
      sptk::PrintErrorMessage("sopr", error_message);
      return 1;
    }
  }

  return 0;
}
