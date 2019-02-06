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

#include <getopt.h>  // getopt_long_only
#include <fstream>   // std::ifstream
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream

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
  *stream << "        they are case-insensitive" << std::endl;
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

}  // namespace

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
          std::ostringstream error_message;
          error_message << "The argument for the -a option must be numeric";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        if (!scalar_operation.AddAdditionOperation(addend)) {
          std::ostringstream error_message;
          error_message << "Failed to add operation by -a option";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        break;
      }
      case 's': {
        double subtrahend;
        if (!sptk::ConvertSpecialStringToDouble(optarg, &subtrahend) &&
            !sptk::ConvertStringToDouble(optarg, &subtrahend)) {
          std::ostringstream error_message;
          error_message << "The argument for the -s option must be numeric";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        if (!scalar_operation.AddSubtractionOperation(subtrahend)) {
          std::ostringstream error_message;
          error_message << "Failed to add operation by -s option";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        break;
      }
      case 'm': {
        double multiplier;
        if (!sptk::ConvertSpecialStringToDouble(optarg, &multiplier) &&
            !sptk::ConvertStringToDouble(optarg, &multiplier)) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be numeric";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        if (!scalar_operation.AddMultiplicationOperation(multiplier)) {
          std::ostringstream error_message;
          error_message << "Failed to add operation by -m option";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        break;
      }
      case 'd': {
        double divisor;
        if (!sptk::ConvertSpecialStringToDouble(optarg, &divisor) &&
            !sptk::ConvertStringToDouble(optarg, &divisor)) {
          std::ostringstream error_message;
          error_message << "The argument for the -d option must be numeric";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        if (!scalar_operation.AddDivisionOperation(divisor)) {
          std::ostringstream error_message;
          error_message << "Failed to add operation by -d option";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        break;
      }
      case 'r': {
        int divisor;
        if (!sptk::ConvertStringToInteger(optarg, &divisor)) {
          std::ostringstream error_message;
          error_message << "The argument for the -r option must be an integer";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        if (!scalar_operation.AddModuloOperation(divisor)) {
          std::ostringstream error_message;
          error_message << "Failed to add operation by -r option";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        break;
      }
      case 'p': {
        double exponent;
        if (!sptk::ConvertSpecialStringToDouble(optarg, &exponent) &&
            !sptk::ConvertStringToDouble(optarg, &exponent)) {
          std::ostringstream error_message;
          error_message << "The argument for the -p option must be numeric";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        if (!scalar_operation.AddPowerOperation(exponent)) {
          std::ostringstream error_message;
          error_message << "Failed to add operation by -p option";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        break;
      }
      case 'l': {
        double lower_bound;
        if (!sptk::ConvertSpecialStringToDouble(optarg, &lower_bound) &&
            !sptk::ConvertStringToDouble(optarg, &lower_bound)) {
          std::ostringstream error_message;
          error_message << "The argument for the -l option must be numeric";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        if (!scalar_operation.AddLowerBoundingOperation(lower_bound)) {
          std::ostringstream error_message;
          error_message << "Failed to add operation by -l option";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        break;
      }
      case 'u': {
        double upper_bound;
        if (!sptk::ConvertSpecialStringToDouble(optarg, &upper_bound) &&
            !sptk::ConvertStringToDouble(optarg, &upper_bound)) {
          std::ostringstream error_message;
          error_message << "The argument for the -u option must be numeric";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        if (!scalar_operation.AddUpperBoundingOperation(upper_bound)) {
          std::ostringstream error_message;
          error_message << "Failed to add operation by -u option";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        break;
      }
      case kABS: {
        if (!scalar_operation.AddAbsoluteOperation()) {
          std::ostringstream error_message;
          error_message << "Failed to add operation by -ABS option";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        break;
      }
      case kINV: {
        if (!scalar_operation.AddReciprocalOperation()) {
          std::ostringstream error_message;
          error_message << "Failed to add operation by -INV option";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        break;
      }
      case kSQR: {
        if (!scalar_operation.AddSquareOperation()) {
          std::ostringstream error_message;
          error_message << "Failed to add operation by -SQR option";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        break;
      }
      case kSQRT: {
        if (!scalar_operation.AddSquareRootOperation()) {
          std::ostringstream error_message;
          error_message << "Failed to add operation by -SQRT option";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        break;
      }
      case kLN: {
        if (!scalar_operation.AddNaturalLogarithmOperation()) {
          std::ostringstream error_message;
          error_message << "Failed to add operation by -LN option";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        break;
      }
      case kLOG2: {
        if (!scalar_operation.AddLogarithmOperation(2.0)) {
          std::ostringstream error_message;
          error_message << "Failed to add operation by -LOG2 option";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        break;
      }
      case kLOG10: {
        if (!scalar_operation.AddLogarithmOperation(10.0)) {
          std::ostringstream error_message;
          error_message << "Failed to add operation by -LOG10 option";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        break;
      }
      case kLOGX: {
        double base;
        if (!sptk::ConvertStringToDouble(optarg, &base)) {
          std::ostringstream error_message;
          error_message << "The argument for the -LOGX option must be numeric";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        if (!scalar_operation.AddLogarithmOperation(base)) {
          std::ostringstream error_message;
          error_message << "Failed to add operation by -LOGX option";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        break;
      }
      case kEXP: {
        if (!scalar_operation.AddNaturalExponentialOperation()) {
          std::ostringstream error_message;
          error_message << "Failed to add operation by -EXP option";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        break;
      }
      case kPOW2: {
        if (!scalar_operation.AddExponentialOperation(2.0)) {
          std::ostringstream error_message;
          error_message << "Failed to add operation by -POW2 option";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        break;
      }
      case kPOW10: {
        if (!scalar_operation.AddExponentialOperation(10.0)) {
          std::ostringstream error_message;
          error_message << "Failed to add operation by -POW10 option";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        break;
      }
      case kPOWX: {
        double base;
        if (!sptk::ConvertStringToDouble(optarg, &base)) {
          std::ostringstream error_message;
          error_message << "The argument for the -POWX option must be numeric";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        if (!scalar_operation.AddExponentialOperation(base)) {
          std::ostringstream error_message;
          error_message << "Failed to add operation by -POWX option";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        break;
      }
      case kFLOOR: {
        if (!scalar_operation.AddFlooringOperation()) {
          std::ostringstream error_message;
          error_message << "Failed to add operation by -FLOOR option";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        break;
      }
      case kCEIL: {
        if (!scalar_operation.AddCeilingOperation()) {
          std::ostringstream error_message;
          error_message << "Failed to add operation by -CEIL option";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        break;
      }
      case kROUND: {
        if (!scalar_operation.AddRoundingOperation()) {
          std::ostringstream error_message;
          error_message << "Failed to add operation by -ROUND option";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        break;
      }
      case kROUNDUP: {
        if (!scalar_operation.AddRoundingUpOperation()) {
          std::ostringstream error_message;
          error_message << "Failed to add operation by -ROUNDUP option";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        break;
      }
      case kROUNDDOWN: {
        if (!scalar_operation.AddRoundingDownOperation()) {
          std::ostringstream error_message;
          error_message << "Failed to add operation by -ROUNDDOWN option";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        break;
      }
      case kUNIT: {
        if (!scalar_operation.AddUnitStepOperation()) {
          std::ostringstream error_message;
          error_message << "Failed to add operation by -UNIT option";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        break;
      }
      case kRAMP: {
        if (!scalar_operation.AddLowerBoundingOperation(0.0)) {
          std::ostringstream error_message;
          error_message << "Failed to add operation by -RAMP option";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        break;
      }
      case kSIGN: {
        if (!scalar_operation.AddSignOperation()) {
          std::ostringstream error_message;
          error_message << "Failed to add operation by -SIGN option";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        break;
      }
      case kSIN: {
        if (!scalar_operation.AddSineOperation()) {
          std::ostringstream error_message;
          error_message << "Failed to add operation by -SIN option";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        break;
      }
      case kCOS: {
        if (!scalar_operation.AddCosineOperation()) {
          std::ostringstream error_message;
          error_message << "Failed to add operation by -COS option";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        break;
      }
      case kTAN: {
        if (!scalar_operation.AddTangentOperation()) {
          std::ostringstream error_message;
          error_message << "Failed to add operation by -TAN option";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        break;
      }
      case kATAN: {
        if (!scalar_operation.AddArctangentOperation()) {
          std::ostringstream error_message;
          error_message << "Failed to add operation by -ATAN option";
          sptk::PrintErrorMessage("sopr", error_message);
          return 1;
        }
        break;
      }
      case kMagic: {
        double magic_number;
        if (!sptk::ConvertSpecialStringToDouble(optarg, &magic_number) &&
            !sptk::ConvertStringToDouble(optarg, &magic_number)) {
          std::ostringstream error_message;
          error_message << "The argument for the -magic option must be numeric";
          sptk::PrintErrorMessage("sopr", error_message);
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
          std::ostringstream error_message;
          error_message << "The argument for the -MAGIC option must be numeric";
          sptk::PrintErrorMessage("sopr", error_message);
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

  // get input file
  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("sopr", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  // open stream
  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("sopr", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  double number;
  while (sptk::ReadStream(&number, &input_stream)) {
    bool is_magic_number;
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
