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

#include <getopt.h>  // getopt_long
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream

#include "SPTK/utils/sptk_utils.h"

namespace {

const int kMagicNumberForInfinity(-1);
const double kDefaultStartValue(0.0);
const double kDefaultStepSize(1.0);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " ramp - generate ramp sequence" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       ramp [ options ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : output length      (   int)[" << std::setw(5) << std::right << "INF"              << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : output order       (   int)[" << std::setw(5) << std::right << "l-1"              << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -s s  : start value        (double)[" << std::setw(5) << std::right << kDefaultStartValue << "][   <= s <=   ]" << std::endl;  // NOLINT
  *stream << "       -e e  : end value          (double)[" << std::setw(5) << std::right << "N/A"              << "][   <= e <=   ]" << std::endl;  // NOLINT
  *stream << "       -t t  : step size          (double)[" << std::setw(5) << std::right << kDefaultStepSize   << "][   <= t <=   ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       ramp sequence              (double)" << std::endl;
  *stream << "  notice:" << std::endl;
  *stream << "       if t = 0.0 and s = e, generate infinite sequence" << std::endl;  // NOLINT
  *stream << "       if 0.0 < t, value of e must be s <= e" << std::endl;
  *stream << "       if t < 0.0, value of e must be e <= s" << std::endl;
  *stream << "       when two or more of -l, -m, or -e are specified," << std::endl;  // NOLINT
  *stream << "       only the argument of the last option is used" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

int main(int argc, char* argv[]) {
  int output_length(kMagicNumberForInfinity);
  double start_value(kDefaultStartValue);
  double end_value(static_cast<double>(kMagicNumberForInfinity));
  double step_size(kDefaultStepSize);
  bool is_end_value_specified(false);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:s:e:t:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &output_length) ||
            output_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("ramp", error_message);
          return 1;
        }
        is_end_value_specified = false;
        break;
      }
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &output_length) ||
            output_length < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                           "non-negative integer";
          sptk::PrintErrorMessage("ramp", error_message);
          return 1;
        }
        ++output_length;
        is_end_value_specified = false;
        break;
      }
      case 's': {
        if (!sptk::ConvertStringToDouble(optarg, &start_value)) {
          std::ostringstream error_message;
          error_message << "The argument for the -s option must be numeric";
          sptk::PrintErrorMessage("ramp", error_message);
          return 1;
        }
        break;
      }
      case 'e': {
        if (!sptk::ConvertStringToDouble(optarg, &end_value)) {
          std::ostringstream error_message;
          error_message << "The argument for the -e option must be numeric";
          sptk::PrintErrorMessage("ramp", error_message);
          return 1;
        }
        is_end_value_specified = true;
        break;
      }
      case 't': {
        if (!sptk::ConvertStringToDouble(optarg, &step_size)) {
          std::ostringstream error_message;
          error_message << "The argument for the -t option must be numeric";
          sptk::PrintErrorMessage("ramp", error_message);
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

  if (0 != argc - optind) {
    std::ostringstream error_message;
    error_message << "Input file is not required";
    sptk::PrintErrorMessage("ramp", error_message);
    return 1;
  }

  if (is_end_value_specified) {
    if (0.0 <= step_size && end_value < start_value) {
      std::ostringstream error_message;
      error_message << "In this case, step size must be negative numeric";
      sptk::PrintErrorMessage("ramp", error_message);
      return 1;
    }
    if (step_size <= 0.0 && start_value < end_value) {
      std::ostringstream error_message;
      error_message << "In this case, step size must be positive numeric";
      sptk::PrintErrorMessage("ramp", error_message);
      return 1;
    }
    if (0.0 == step_size && start_value == end_value) {
      output_length = kMagicNumberForInfinity;
    } else {
      output_length =
          static_cast<int>((end_value - start_value) / step_size) + 1;
    }
  }

  double output(start_value);
  for (int i(0); kMagicNumberForInfinity == output_length || i < output_length;
       ++i) {
    if (!sptk::WriteStream(output, &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write ramp sequence";
      sptk::PrintErrorMessage("ramp", error_message);
      return 1;
    }
    output += step_size;
  }

  return 0;
}
