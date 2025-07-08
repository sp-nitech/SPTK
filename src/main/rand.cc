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

#include "GETOPT/ya_getopt.h"
#include "SPTK/generation/uniform_distributed_random_value_generation.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kMagicNumberForInfinity(-1);
const int kDefaultSeed(1);
const double kDefaultLowerBound(0.0);
const double kDefaultUpperBound(1.0);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " rand - generate uniform distributed random value" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       rand [ options ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : output length      (   int)[" << std::setw(5) << std::right << "INF"              << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : output order       (   int)[" << std::setw(5) << std::right << "l-1"              << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -s s  : seed               (   int)[" << std::setw(5) << std::right << kDefaultSeed       << "][   <= s <=   ]" << std::endl;  // NOLINT
  *stream << "       -a a  : lower bound        (double)[" << std::setw(5) << std::right << kDefaultLowerBound << "][   <= a <=   ]" << std::endl;  // NOLINT
  *stream << "       -b b  : upper bound        (double)[" << std::setw(5) << std::right << kDefaultUpperBound << "][   <= b <=   ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       random values              (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a rand [ @e option ]
 *
 * - @b -l @e int
 *   - output length @f$(1 \le L)@f$
 * - @b -m @e int
 *   - output order @f$(0 \le L - 1)@f$
 * - @b -s @e int
 *   - random seed
 * - @b -a @e double
 *   - lower bound @f$(a < b)@f$
 * - @b -b @e double
 *   - upper bound @f$(b > a)@f$
 * - @b stdout
 *   - double-type random values
 *
 * The output of this command is
 * @f[
 *   \begin{array}{ccccc}
 *     \epsilon(0), & \epsilon(1), & \ldots, & \epsilon(L - 1)
 *   \end{array}
 * @f]
 * where
 * @f[
 *   \epsilon(l) \sim \mathcal{U}(a, b).
 * @f]
 * If the output length @f$L@f$ is not given, an infinite random value sequence
 * is generated.
 *
 * In the below example, uniform distributed random values of length 100 are
 * generated:
 *
 * @code{.sh}
 *   rand -l 100 > data.rnd
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int output_length(kMagicNumberForInfinity);
  int seed(kDefaultSeed);
  double lower_bound(kDefaultLowerBound);
  double upper_bound(kDefaultUpperBound);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:s:a:b:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &output_length) ||
            output_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("rand", error_message);
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
          sptk::PrintErrorMessage("rand", error_message);
          return 1;
        }
        ++output_length;
        break;
      }
      case 's': {
        if (!sptk::ConvertStringToInteger(optarg, &seed)) {
          std::ostringstream error_message;
          error_message << "The argument for the -s option must be an integer";
          sptk::PrintErrorMessage("rand", error_message);
          return 1;
        }
        break;
      }
      case 'a': {
        if (!sptk::ConvertStringToDouble(optarg, &lower_bound)) {
          std::ostringstream error_message;
          error_message << "The argument for the -a option must be numeric";
          sptk::PrintErrorMessage("rand", error_message);
          return 1;
        }
        break;
      }
      case 'b': {
        if (!sptk::ConvertStringToDouble(optarg, &upper_bound)) {
          std::ostringstream error_message;
          error_message << "The argument for the -b option must be numeric";
          sptk::PrintErrorMessage("rand", error_message);
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
    sptk::PrintErrorMessage("rand", error_message);
    return 1;
  }

  if (!sptk::SetBinaryMode()) {
    std::ostringstream error_message;
    error_message << "Cannot set translation mode";
    sptk::PrintErrorMessage("rand", error_message);
    return 1;
  }

  sptk::UniformDistributedRandomValueGeneration generator(seed, lower_bound,
                                                          upper_bound);

  for (int i(0); kMagicNumberForInfinity == output_length || i < output_length;
       ++i) {
    double random;
    if (!generator.Get(&random)) {
      std::ostringstream error_message;
      error_message << "Failed to generate random values";
      sptk::PrintErrorMessage("rand", error_message);
      return 1;
    }
    if (!sptk::WriteStream(random, &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write random values";
      sptk::PrintErrorMessage("rand", error_message);
      return 1;
    }
  }

  return 0;
}
