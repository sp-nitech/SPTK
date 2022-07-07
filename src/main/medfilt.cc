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
#include "SPTK/filter/median_filter.h"
#include "SPTK/input/input_source_from_stream.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum LongOptions {
  kMagic = 1000,
};

enum WaysToApplyFilter {
  kEachDimension = 0,
  kAcrossDimension,
  kNumWaysToApplyFilter
};

const int kDefaultNumInputOrder(0);
const int kDefaultNumFilterOrder(2);
const WaysToApplyFilter kDefaultWayToApplyFilter(kEachDimension);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " medfilt - median filter" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       medfilt [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l          : length of vector        (   int)[" << std::setw(5) << std::right << kDefaultNumInputOrder + 1 << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m          : order of vector         (   int)[" << std::setw(5) << std::right << "l-1"                     << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -k k          : order of filter         (   int)[" << std::setw(5) << std::right << kDefaultNumFilterOrder    << "][ 0 <= k <=   ]" << std::endl;  // NOLINT
  *stream << "       -w w          : way to apply filter     (   int)[" << std::setw(5) << std::right << kDefaultWayToApplyFilter  << "][ 0 <= w <= 1 ]" << std::endl;  // NOLINT
  *stream << "                         0 (each dimension)" << std::endl;
  *stream << "                         1 (across dimension)" << std::endl;
  *stream << "       -magic magic  : magic number            (double)[" << std::setw(5) << std::right << "N/A"                     << "]" << std::endl;  // NOLINT
  *stream << "       -h            : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence                           (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       filtered data sequence                  (double)" << std::endl;  // NOLINT
  *stream << "  notice:" << std::endl;
  *stream << "       if w = 0, output size is m+1, otherwise 1" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a medfilt [ @e option ] [ @e infile ]
 *
 * - @b -l @e int
 *   - length of vector @f$(1 \le M + 1)@f$
 * - @b -m @e int
 *   - order of vector @f$(0 \le M)@f$
 * - @b -k @e int
 *   - order of filter @f$(0 \le K)@f$
 * - @b -w @e int
 *   - way to apply filter
 *     \arg @c 0 each dimension
 *     \arg @c 1 across dimension
 * - @b -magic @e double
 *   - magic number
 * - @b infile @e str
 *   - double-type data sequence
 * - @b stdout
 *   - double-type filtered data sequence
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_input_order(kDefaultNumInputOrder);
  int num_filter_order(kDefaultNumFilterOrder);
  WaysToApplyFilter way_to_apply_filter(kDefaultWayToApplyFilter);
  double magic_number(0.0);
  bool is_magic_number_specified(false);

  const struct option long_options[] = {
      {"magic", required_argument, NULL, kMagic},
      {0, 0, 0, 0},
  };

  for (;;) {
    const int option_char(
        getopt_long_only(argc, argv, "l:m:k:w:h", long_options, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &num_input_order) ||
            num_input_order <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("medfilt", error_message);
          return 1;
        }
        --num_input_order;
        break;
      }
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_input_order) ||
            num_input_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("medfilt", error_message);
          return 1;
        }
        break;
      }
      case 'k': {
        if (!sptk::ConvertStringToInteger(optarg, &num_filter_order) ||
            num_filter_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -k option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("medfilt", error_message);
          return 1;
        }
        break;
      }
      case 'w': {
        const int min(0);
        const int max(static_cast<int>(kNumWaysToApplyFilter) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -w option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("medfilt", error_message);
          return 1;
        }
        way_to_apply_filter = static_cast<WaysToApplyFilter>(tmp);
        break;
      }
      case kMagic: {
        if (!sptk::ConvertStringToDouble(optarg, &magic_number)) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -magic option must be a number";
          sptk::PrintErrorMessage("medfilt", error_message);
          return 1;
        }
        is_magic_number_specified = true;
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
    sptk::PrintErrorMessage("medfilt", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("medfilt", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  const int input_length(num_input_order + 1);
  sptk::InputSourceFromStream input_source(false, input_length, &input_stream);
  sptk::MedianFilter median_filter(num_input_order, num_filter_order,
                                   &input_source,
                                   kEachDimension == way_to_apply_filter,
                                   is_magic_number_specified, magic_number);
  if (!median_filter.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize MedianFilter";
    sptk::PrintErrorMessage("medfilt", error_message);
    return 1;
  }

  const int output_length(median_filter.GetSize());
  std::vector<double> output(output_length);

  while (median_filter.Get(&output)) {
    if (!sptk::WriteStream(0, output_length, output, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write output";
      sptk::PrintErrorMessage("medfilt", error_message);
      return 1;
    }
  }

  return 0;
}
