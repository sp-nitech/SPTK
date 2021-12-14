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

#include <algorithm>  // std::transform
#include <fstream>    // std::ifstream
#include <iomanip>    // std::setw
#include <iostream>   // std::cerr, std::cin, std::cout, std::endl, etc.
#include <numeric>    // std::accumulate
#include <sstream>    // std::ostringstream
#include <vector>     // std::vector

#include "Getopt/getoptwin.h"
#include "SPTK/math/histogram_calculation.h"
#include "SPTK/math/statistics_accumulation.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kMagicNumberForEndOfFile(-1);
const int kDefaultNumBin(10);
const double kDefaultLowerBound(0.0);
const double kDefaultUpperBound(1.0);
const bool kDefaultNormalizationFlag(false);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " histogram - calculate histogram" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       histogram [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -t t  : output interval    (   int)[" << std::setw(5) << std::right << "EOF"              << "][ 1 <= t <=   ]" << std::endl;  // NOLINT
  *stream << "       -b b  : number of bins     (   int)[" << std::setw(5) << std::right << kDefaultNumBin     << "][ 1 <= b <=   ]" << std::endl;  // NOLINT
  *stream << "       -l l  : lower bound        (double)[" << std::setw(5) << std::right << kDefaultLowerBound << "][   <= l <  u ]" << std::endl;  // NOLINT
  *stream << "       -u u  : upper bound        (double)[" << std::setw(5) << std::right << kDefaultUpperBound << "][ l <  u <=   ]" << std::endl;  // NOLINT
  *stream << "       -n    : normalization      (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultNormalizationFlag) << "]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence              (double)[stdin]" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       histogram                  (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a histogram [ @e option ] [ @e infile ]
 *
 * - @b -t @e int
 *   - output interval @f$(1 \le T)@f$
 * - @b -b @e int
 *   - number of bins @f$(1 \le N)@f$
 * - @b -l @e double
 *   - lower bound @f$(y_L < y_U)@f$
 * - @b -u @e double
 *   - upper bound @f$(y_L < y_U)@f$
 * - @b -n @e bool
 *   - perform normalization
 * - @b infile @e str
 *   - double-type data sequence
 * - @b stdout
 *   - double-type histogram
 *
 * @code{.sh}
 *   # 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
 *   ramp -l 10 | histogram -b 4 -l 0 -u 9 | x2x +da
 *   # 3, 2, 2, 3
 *   ramp -l 10 | histogram -b 4 -l 0 -u 9 -n | x2x +da
 *   # 0.3, 0.2, 0.2, 0.3
 *   ramp -l 10 | histogram -b 4 -l 0 -u 9 -t 5 | x2x +da
 *   # 3, 2, 0, 0, 0, 0, 2, 3
 * @endcode
 */
int main(int argc, char* argv[]) {
  int output_interval(kMagicNumberForEndOfFile);
  int num_bin(kDefaultNumBin);
  double lower_bound(kDefaultLowerBound);
  double upper_bound(kDefaultUpperBound);
  bool normalization_flag(kDefaultNormalizationFlag);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "t:b:l:u:nh", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 't': {
        if (!sptk::ConvertStringToInteger(optarg, &output_interval) ||
            output_interval <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -t option must be a positive integer";
          sptk::PrintErrorMessage("histogram", error_message);
          return 1;
        }
        break;
      }
      case 'b': {
        if (!sptk::ConvertStringToInteger(optarg, &num_bin) || num_bin <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -b option must be a positive integer";
          sptk::PrintErrorMessage("histogram", error_message);
          return 1;
        }
        break;
      }
      case 'l': {
        if (!sptk::ConvertStringToDouble(optarg, &lower_bound)) {
          std::ostringstream error_message;
          error_message << "The argument for the -l option must be numeric";
          sptk::PrintErrorMessage("histogram", error_message);
          return 1;
        }
        break;
      }
      case 'u': {
        if (!sptk::ConvertStringToDouble(optarg, &upper_bound)) {
          std::ostringstream error_message;
          error_message << "The argument for the -u option must be numeric";
          sptk::PrintErrorMessage("histogram", error_message);
          return 1;
        }
        break;
      }
      case 'n': {
        normalization_flag = true;
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

  if (upper_bound <= lower_bound) {
    std::ostringstream error_message;
    error_message << "Upper bound must be greater than lower bound";
    sptk::PrintErrorMessage("histogram", error_message);
    return 1;
  }

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("histogram", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("histogram", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::HistogramCalculation histogram_calculation(num_bin, lower_bound,
                                                   upper_bound);
  if (!histogram_calculation.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize HistogramCalculation";
    sptk::PrintErrorMessage("histogram", error_message);
    return 1;
  }

  std::vector<double> histogram(num_bin);

  if (kMagicNumberForEndOfFile == output_interval) {
    std::vector<double> data(1);
    sptk::StatisticsAccumulation statistics_accumulation(num_bin - 1, 1);
    sptk::StatisticsAccumulation::Buffer buffer;
    while (sptk::ReadStream(false, 0, 0, 1, &data, &input_stream, NULL)) {
      if (!histogram_calculation.Run(data, &histogram)) {
        std::ostringstream error_message;
        error_message << "Failed to calculate histogram";
        sptk::PrintErrorMessage("histogram", error_message);
        return 1;
      }
      if (!statistics_accumulation.Run(histogram, &buffer)) {
        std::ostringstream error_message;
        error_message << "Failed to accumulate histogram";
        sptk::PrintErrorMessage("histogram", error_message);
        return 1;
      }
    }

    if (!statistics_accumulation.GetSum(buffer, &histogram)) {
      std::ostringstream error_message;
      error_message << "Failed to get histogram";
      sptk::PrintErrorMessage("histogram", error_message);
      return 1;
    }

    if (normalization_flag) {
      const double sum(
          std::accumulate(histogram.begin(), histogram.end(), 0.0));
      if (0.0 == sum) {
        std::ostringstream error_message;
        error_message << "Failed to calculate normalized histogram";
        sptk::PrintErrorMessage("histogram", error_message);
        return 1;
      }
      const double z(1.0 / sum);
      std::transform(histogram.begin(), histogram.end(), histogram.begin(),
                     [z](double x) { return x * z; });
    }

    if (!sptk::WriteStream(0, num_bin, histogram, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write histogram";
      sptk::PrintErrorMessage("histogram", error_message);
      return 1;
    }
  } else {
    std::vector<double> data(output_interval);
    for (int frame_index(0); sptk::ReadStream(false, 0, 0, output_interval,
                                              &data, &input_stream, NULL);
         ++frame_index) {
      if (!histogram_calculation.Run(data, &histogram)) {
        std::ostringstream error_message;
        error_message << "Failed to calculate histogram";
        sptk::PrintErrorMessage("histogram", error_message);
        return 1;
      }

      if (normalization_flag) {
        const double sum(
            std::accumulate(histogram.begin(), histogram.end(), 0.0));
        if (0.0 == sum) {
          std::ostringstream error_message;
          error_message << "Failed to calculate normalized histogram at "
                        << frame_index << "th frame";
          sptk::PrintErrorMessage("histogram", error_message);
          return 1;
        }
        const double z(1.0 / sum);
        std::transform(histogram.begin(), histogram.end(), histogram.begin(),
                       [z](double x) { return x * z; });
      }

      if (!sptk::WriteStream(0, num_bin, histogram, &std::cout, NULL)) {
        std::ostringstream error_message;
        error_message << "Failed to write histogram";
        sptk::PrintErrorMessage("histogram", error_message);
        return 1;
      }
    }
  }

  return 0;
}
