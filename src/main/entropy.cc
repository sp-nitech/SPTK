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
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream
#include <vector>    // std::vector

#include "Getopt/getoptwin.h"
#include "SPTK/math/entropy_calculation.h"
#include "SPTK/math/statistics_accumulation.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultNumElement(256);
const sptk::EntropyCalculation::EntropyUnits kDefaultEntropyUnit(
    sptk::EntropyCalculation::EntropyUnits::kBit);
const bool kDefaultOutputFrameByFrameFlag(false);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " entropy - calculate entropy" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       entropy [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : number of elements    (   int)[" << std::setw(5) << std::right << kDefaultNumElement  << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -o o  : output format         (   int)[" << std::setw(5) << std::right << kDefaultEntropyUnit << "][ 0 <= o <= 2 ]" << std::endl;  // NOLINT
  *stream << "                 0 (bit)" << std::endl;
  *stream << "                 1 (nat)" << std::endl;
  *stream << "                 2 (dit)" << std::endl;
  *stream << "       -f    : output frame by frame (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultOutputFrameByFrameFlag) << "]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       probability sequence          (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       entropy                       (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a entropy [ @e option ] [ @e infile ]
 *
 * - @b -l @e int
 *   - number of elements @f$(1 \le N)@f$
 * - @b -o @e int
 *   - output format @f$O@f$
 *     \arg @c 0 bit
 *     \arg @c 1 nat
 *     \arg @c 2 dit
 * - @b -f @e bool
 *   - output entropy frame by frame
 * - @b infile @e str
 *   - double-type probability sequence
 * - @b stdout
 *   - double-type entropy
 *
 * The input is a set of probabilities:
 * @f[
 *   \begin{array}{ccc}
 *     \underbrace{p_0(1),\,p_0(2),\,\ldots,\,p_0(N)}_{\boldsymbol{p}(0)}, &
 *     \underbrace{p_1(1),\,p_1(2),\,\ldots,\,p_1(N)}_{\boldsymbol{p}(1)}, &
 *     \ldots,
 *   \end{array}
 * @f]
 * If @c -f option is given, the output sequence is
 * @f[
 *   \begin{array}{cccc}
 *     H(0), & H(1), & H(2), & \ldots,
 *   \end{array}
 * @f]
 * where @f$H(t)@f$ is the entropy at @f$t@f$-th frame:
 * @f[
 *   H(t) = -\sum_{n=1}^{N} p_t(n) \log_b p_t(n)
 *        = -\boldsymbol{p}(t)^\mathsf{T} \log_b \boldsymbol{p}(t).
 * @f]
 * The base @f$b@f$ depends on the value of @f$O@f$:
 * @f[
 *   b = \left\{\begin{array}{ll}
 *     2,\quad & O = 0 \\
 *     e,      & O = 1 \\
 *     10.     & O = 2 \\
 *   \end{array}\right.
 * @f]
 * If @c -f option is not given, only the average of the entropies,
 * @f$\bar{H}@f$, is sent to the standard output:
 * @f[
 *   \bar{H} = \frac{1}{T} \sum_{t=0}^{T-1} H(t),
 * @f]
 * where @f$T@f$ is the number of the set of probabilities.
 *
 * The below example calculates maximum value of entropy:
 *
 * @code{.sh}
 *   step -l 4 | sopr -d 4 | entropy -l 4 | x2x +da
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_element(kDefaultNumElement);
  sptk::EntropyCalculation::EntropyUnits entropy_unit(kDefaultEntropyUnit);
  bool output_frame_by_frame(kDefaultOutputFrameByFrameFlag);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:o:fh", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &num_element) ||
            num_element <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("entropy", error_message);
          return 1;
        }
        break;
      }
      case 'o': {
        const int min(0);
        const int max(static_cast<int>(
                          sptk::EntropyCalculation::EntropyUnits::kNumUnits) -
                      1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -o option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("entropy", error_message);
          return 1;
        }
        entropy_unit = static_cast<sptk::EntropyCalculation::EntropyUnits>(tmp);
        break;
      }
      case 'f': {
        output_frame_by_frame = true;
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
    sptk::PrintErrorMessage("entropy", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("entropy", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::StatisticsAccumulation statistics_accumulation(0, 1);
  sptk::StatisticsAccumulation::Buffer buffer;
  sptk::EntropyCalculation entropy_calculation(num_element, entropy_unit);
  if (!statistics_accumulation.IsValid() || !entropy_calculation.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize EntropyCalculation";
    sptk::PrintErrorMessage("entropy", error_message);
    return 1;
  }

  std::vector<double> probability(num_element);
  double entropy;

  while (sptk::ReadStream(false, 0, 0, num_element, &probability, &input_stream,
                          NULL)) {
    if (!entropy_calculation.Run(probability, &entropy)) {
      std::ostringstream error_message;
      error_message << "Failed to calculate entropy";
      sptk::PrintErrorMessage("entropy", error_message);
      return 1;
    }

    if (output_frame_by_frame) {
      if (!sptk::WriteStream(entropy, &std::cout)) {
        std::ostringstream error_message;
        error_message << "Failed to write entropy";
        sptk::PrintErrorMessage("entropy", error_message);
        return 1;
      }
    } else {
      if (!statistics_accumulation.Run(std::vector<double>{entropy}, &buffer)) {
        std::ostringstream error_message;
        error_message << "Failed to accumulate statistics";
        sptk::PrintErrorMessage("entropy", error_message);
        return 1;
      }
    }
  }

  int num_data;
  if (!statistics_accumulation.GetNumData(buffer, &num_data)) {
    std::ostringstream error_message;
    error_message << "Failed to accumulate statistics";
    sptk::PrintErrorMessage("entropy", error_message);
    return 1;
  }

  if (!output_frame_by_frame && 0 < num_data) {
    std::vector<double> average_entropy(1);
    if (!statistics_accumulation.GetMean(buffer, &average_entropy)) {
      std::ostringstream error_message;
      error_message << "Failed to calculate entropy";
      sptk::PrintErrorMessage("entropy", error_message);
      return 1;
    }
    if (!sptk::WriteStream(0, 1, average_entropy, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write entropy";
      sptk::PrintErrorMessage("entropy", error_message);
      return 1;
    }
  }

  return 0;
}
