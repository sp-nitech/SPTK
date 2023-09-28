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

#include <cmath>     // std::cos, std::sin
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream

#include "GETOPT/ya_getopt.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum OutputFormats { kSine = 0, kCosine, kNumOutputFormats };

const int kMagicNumberForInfinity(-1);
const double kDefaultPeriod(10.0);
const double kDefaultAmplitude(1.0);
const OutputFormats kDefaultOutputFormat(kSine);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " sin - generate sinusoidal sequence" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       sin [ options ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : output length      (   int)[" << std::setw(5) << std::right << "INF"                << "][   1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : output order       (   int)[" << std::setw(5) << std::right << "l-1"                << "][   0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -p p  : period             (double)[" << std::setw(5) << std::right << kDefaultPeriod       << "][ 0.0 <  p <=   ]" << std::endl;  // NOLINT
  *stream << "       -a a  : amplitude          (double)[" << std::setw(5) << std::right << kDefaultAmplitude    << "][     <= a <=   ]" << std::endl;  // NOLINT
  *stream << "       -o o  : output format      (   int)[" << std::setw(5) << std::right << kDefaultOutputFormat << "][   0 <= o <= 1 ]" << std::endl;  // NOLINT
  *stream << "                 0 (sine)" << std::endl;
  *stream << "                 1 (cosine)" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       sinusoidal sequence        (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a sin [ @e option ]
 *
 * - @b -l @e int
 *   - output length @f$(1 \le L)@f$
 * - @b -m @e int
 *   - output order @f$(0 \le L - 1)@f$
 * - @b -p @e double
 *   - period @f$(0 < P)@f$
 * - @b -a @e double
 *   - amplitude @f$(A)@f$
 * - @b -o @e int
 *   - output format
 *     \arg @c 0 sine
 *     \arg @c 1 cosine
 * - @b stdout
 *   - double-type sinusoidal sequence
 *
 * The output of this command is
 * @f[
 *   \begin{array}{cccc}
 *     x(0), & x(1), & \ldots, & x(L-1),
 *   \end{array}
 * @f]
 * where
 * @f[
 *   x(l) = A \sin \left( \frac{2\pi l}{P} \right).
 * @f]
 * If @f$L@f$ is not given, an infinite sinusoidal sequence is generated.
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int output_length(kMagicNumberForInfinity);
  double period(kDefaultPeriod);
  double amplitude(kDefaultAmplitude);
  OutputFormats output_format(kDefaultOutputFormat);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:p:a:o:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &output_length) ||
            output_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("sin", error_message);
          return 1;
        }
        break;
      }
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &output_length) ||
            output_length < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                           "non-negative integer";
          sptk::PrintErrorMessage("sin", error_message);
          return 1;
        }
        ++output_length;
        break;
      }
      case 'p': {
        if (!sptk::ConvertStringToDouble(optarg, &period) || period <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -p option must be a positive number";
          sptk::PrintErrorMessage("sin", error_message);
          return 1;
        }
        break;
      }
      case 'a': {
        if (!sptk::ConvertStringToDouble(optarg, &amplitude)) {
          std::ostringstream error_message;
          error_message << "The argument for the -a option must be numeric";
          sptk::PrintErrorMessage("sin", error_message);
          return 1;
        }
        break;
      }
      case 'o': {
        const int min(0);
        const int max(static_cast<int>(kNumOutputFormats) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -o option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("sin", error_message);
          return 1;
        }
        output_format = static_cast<OutputFormats>(tmp);
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
    sptk::PrintErrorMessage("sin", error_message);
    return 1;
  }

  if (!sptk::SetBinaryMode()) {
    std::ostringstream error_message;
    error_message << "Cannot set translation mode";
    sptk::PrintErrorMessage("sin", error_message);
    return 1;
  }

  const double omega(sptk::kTwoPi / period);
  for (int i(0); kMagicNumberForInfinity == output_length || i < output_length;
       ++i) {
    double output;
    switch (output_format) {
      case kSine: {
        output = amplitude * std::sin(omega * i);
        break;
      }
      case kCosine: {
        output = amplitude * std::cos(omega * i);
        break;
      }
      default: {
        return 1;
      }
    }
    if (!sptk::WriteStream(output, &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write sinusoidal sequence";
      sptk::PrintErrorMessage("sin", error_message);
      return 1;
    }
  }

  return 0;
}
