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

#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream

#include "Getopt/getoptwin.h"
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

/**
 * @a ramp [ @e option ]
 *
 * - @b -l @e int
 *   - output length @f$(1 \le L)@f$
 * - @b -m @e int
 *   - output order @f$(0 \le L - 1)@f$
 * - @b -s @e double
 *   - start value @f$(S)@f$
 * - @b -e @e double
 *   - end value @f$(E)@f$
 * - @b -t @e double
 *   - step size @f$(T)@f$
 * - @b stdout
 *   - double-type ramp sequence
 *
 * The output of this command is
 * @f[
 *   \begin{array}{cccc}
 *     x(0), & x(1), & \ldots, & x(L-1),
 *   \end{array}
 * @f]
 * where
 * @f[
 *   x(l) = S + Tl.
 * @f]
 * If @f$E@f$ rather than @f$L@f$ is given, the output is
 * @f[
 *   \begin{array}{cccc}
 *     x(0), & x(1), & \ldots, & x((E-S)/T).
 *   \end{array}
 * @f]
 * If @f$L@f$ is not given, an infinite ramp sequence is generated.
 * If @f$S=E@f$ and @f$T=0@f$, an infinite step sequence is generated.
 *
 * The below example outputs a parabolic sequence.
 *
 * @code{.sh}
 *   ramp -l 256 | sopr -SQR | dmp +d
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
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
