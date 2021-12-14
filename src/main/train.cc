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

#include <cmath>     // std::sqrt
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream

#include "Getopt/getoptwin.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum NormalizationType {
  kNone = 0,
  kPower,
  kMagnitude,
  kNumNormalizationTypes
};

const int kMagicNumberForInfinity(-1);
const double kDefaultPeriod(10.0);
const NormalizationType kDefaultNormalizationType(NormalizationType::kPower);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " train - generate pulse sequence" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       train [ options ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : output length      (   int)[" << std::setw(5) << std::right << "INF"                     << "][   1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : output order       (   int)[" << std::setw(5) << std::right << "l-1"                     << "][   0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -p p  : frame period       (double)[" << std::setw(5) << std::right << kDefaultPeriod            << "][ 1.0 <= p <=   ]" << std::endl;  // NOLINT
  *stream << "       -n n  : normalization type (   int)[" << std::setw(5) << std::right << kDefaultNormalizationType << "][   0 <= n <= 2 ]" << std::endl;  // NOLINT
  *stream << "                 0 (none)" << std::endl;
  *stream << "                 1 (power)" << std::endl;
  *stream << "                 2 (magnitude)" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       pulse sequence             (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a train [ @e option ]
 *
 * - @b -l @e int
 *   - output length @f$(1 \le L)@f$
 * - @b -m @e int
 *   - output order @f$(0 \le L - 1)@f$
 * - @b -p @e double
 *   - frame period @f$(1 \le P)@f$
 * - @b -n @e int
 *   - normalization type @f$(0 \le N \le 2)@f$
 *     \arg @c 0 none
 *     \arg @c 1 power
 *     \arg @c 2 magnitude
 * - @b stdout
 *   - double-type pulse sequence
 *
 * The output of this command is
 * @f[
 *   \begin{array}{cccc}
 *     x(0), & x(1), & \ldots, & x(L-1)
 *   \end{array}
 * @f]
 * where @f$x(l)@f$ is non-zero at every @f$P@f$ period.
 * If @f$L@f$ is not given, an infinite pulse sequence is generated.
 *
 * There are three kind of normalization types:
 * @f{eqnarray}{
 *   \sum_{l=0}^{P-1} x(l+a) &=& 1, \quad (N=0) \\
 *   \frac{1}{P} \sum_{l=0}^{P-1} x^2(l+a) &=& 1, \quad (N=1) \\
 *   \frac{1}{P} \sum_{l=0}^{P-1} x(l+a) &=& 1, \quad (N=2)
 * @f}
 * where @f$a@f$ is any index.
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int output_length(kMagicNumberForInfinity);
  double period(kDefaultPeriod);
  NormalizationType normalization_type(kDefaultNormalizationType);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:p:n:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &output_length) ||
            output_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("train", error_message);
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
          sptk::PrintErrorMessage("train", error_message);
          return 1;
        }
        ++output_length;
        break;
      }
      case 'p': {
        if (!sptk::ConvertStringToDouble(optarg, &period) || period < 1.0) {
          std::ostringstream error_message;
          error_message << "The argument for the -p option must be equal to or "
                           "greater than 1.0";
          sptk::PrintErrorMessage("train", error_message);
          return 1;
        }
        break;
      }
      case 'n': {
        const int min(0);
        const int max(
            static_cast<int>(NormalizationType::kNumNormalizationTypes) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -n option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("train", error_message);
          return 1;
        }
        normalization_type = static_cast<NormalizationType>(tmp);
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
    sptk::PrintErrorMessage("train", error_message);
    return 1;
  }

  double pulse;
  switch (normalization_type) {
    case kNone: {
      pulse = 1.0;
      break;
    }
    case kPower: {
      pulse = std::sqrt(period);
      break;
    }
    case kMagnitude: {
      pulse = period;
      break;
    }
    default: {
      return 1;
    }
  }

  const double frequency(1.0 / period);
  double phase(1.0);
  for (int i(0); kMagicNumberForInfinity == output_length || i < output_length;
       ++i) {
    if (1.0 <= phase) {
      phase += frequency - 1.0;
      if (!sptk::WriteStream(pulse, &std::cout)) {
        std::ostringstream error_message;
        error_message << "Failed to write pulse sequence";
        sptk::PrintErrorMessage("train", error_message);
        return 1;
      }
    } else {
      phase += frequency;
      if (!sptk::WriteStream(0.0, &std::cout)) {
        std::ostringstream error_message;
        error_message << "Failed to write pulse sequence";
        sptk::PrintErrorMessage("train", error_message);
        return 1;
      }
    }
  }

  return 0;
}
