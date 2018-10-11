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
//                1996-2018  Nagoya Institute of Technology          //
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

#include <getopt.h>  // getopt_long
#include <cfloat>    // DBL_MAX
#include <fstream>   // std::ifstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream

#include "SPTK/math/scalar_operation.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const double kDefaultLowerBound(-DBL_MAX);
const double kDefaultUpperBound(DBL_MAX);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " clip - data clipping" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       clip [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : lower bound        (double)[" << std::setw(5) << std::right << "-INF" << "][   <= l <= u ]" << std::endl;  // NOLINT
  *stream << "       -u u  : upper bound        (double)[" << std::setw(5) << std::right <<  "INF" << "][ l <= u <=   ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence              (double)[stdin]" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       clipped data sequence      (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

int main(int argc, char* argv[]) {
  double lower_bound(kDefaultLowerBound);
  double upper_bound(kDefaultUpperBound);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:u:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToDouble(optarg, &lower_bound)) {
          std::ostringstream error_message;
          error_message << "The argument for the -l option must be numeric";
          sptk::PrintErrorMessage("clip", error_message);
          return 1;
        }
        break;
      }
      case 'u': {
        if (!sptk::ConvertStringToDouble(optarg, &upper_bound)) {
          std::ostringstream error_message;
          error_message << "The argument for the -u option must be numeric";
          sptk::PrintErrorMessage("clip", error_message);
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

  if (upper_bound < lower_bound) {
    std::ostringstream error_message;
    error_message << "Upper bound must be equal to or greater than lower bound";
    sptk::PrintErrorMessage("clip", error_message);
    return 1;
  }

  // get input file
  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("clip", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  // open stream
  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("clip", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::ScalarOperation scalar_operation;
  if (-DBL_MAX != lower_bound &&
      !scalar_operation.AddLowerBoundingOperation(lower_bound)) {
    std::ostringstream error_message;
    error_message << "Failed to add lower bounding operation";
    sptk::PrintErrorMessage("clip", error_message);
    return 1;
  }
  if (DBL_MAX != upper_bound &&
      !scalar_operation.AddUpperBoundingOperation(upper_bound)) {
    std::ostringstream error_message;
    error_message << "Failed to add upper bounding operation";
    sptk::PrintErrorMessage("clip", error_message);
    return 1;
  }

  double data;
  while (sptk::ReadStream(&data, &input_stream)) {
    bool is_magic_number;
    if (!scalar_operation.Run(&data, &is_magic_number)) {
      std::ostringstream error_message;
      error_message << "Failed to clip";
      sptk::PrintErrorMessage("clip", error_message);
      return 1;
    }
    if (!sptk::WriteStream(data, &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write data";
      sptk::PrintErrorMessage("clip", error_message);
      return 1;
    }
  }

  return 0;
}
