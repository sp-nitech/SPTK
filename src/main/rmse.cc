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
#include <fstream>   // std::ifstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream
#include <vector>    // std::vector

#include "Getopt/getoptwin.h"
#include "SPTK/math/statistics_accumulation.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum LongOptions {
  kMagic = 1000,
};

const int kMagicNumberForEndOfFile(-1);
const bool kDefaultUseMagicNumber(false);
const bool kDefaultOutputFrameByFrameFlag(false);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " rmse - calculation of root mean squared error" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       rmse [ options ] file1 [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l         : length of vector      (   int)[" << std::setw(5) << std::right << "EOF" << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m         : order of vector       (   int)[" << std::setw(5) << std::right << "l-1" << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -magic magic : remove magic number   (double)[" << std::setw(5) << std::right << "N/A" << "]" << std::endl;  // NOLINT
  *stream << "       -f           : output frame by frame (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultOutputFrameByFrameFlag) << "]" << std::endl;  // NOLINT
  *stream << "       -h           : print this message" << std::endl;
  *stream << "  file1:" << std::endl;
  *stream << "       data sequence                        (double)" << std::endl;  // NOLINT
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence                        (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       root mean squared error              (double)" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a rmse [ @e option ] @e file1 [ @e infile ]
 *
 * - @b -l @e int
 *   - length of vector @f$(1 \le L)@f$
 * - @b -m @e int
 *   - order of vector @f$(0 \le L - 1)@f$
 * - @b -magic @e double
 *   - remove magic number
 * - @b -f @e bool
 *   - output RMSE frame-by-frame
 * - @b file1 @e str
 *   - double-type data sequence
 * - @b infile @e str
 *   - double-type data sequence
 * - @b stdout
 *   - double-type RMSE
 *
 * The inputs of this command are
 * @f[
 *   \begin{array}{ccc}
 *     \underbrace{x_0(0), \; \ldots, \; x_0(L-1)}_L, &
 *     \underbrace{x_1(0), \; \ldots, \; x_1(L-1)}_L, &
 *     \ldots,
 *   \end{array}
 * @f]
 * and
 * @f[
 *   \begin{array}{ccc}
 *     \underbrace{y_0(0), \; \ldots, \; y_0(L-1)}_L, &
 *     \underbrace{y_1(0), \; \ldots, \; y_1(L-1)}_L, &
 *     \ldots,
 *   \end{array}
 * @f]
 * and the output is the sequence of RMSE:
 * @f[
 *   \begin{array}{ccc}
 *     e_0, & e_1, & \ldots,
 *   \end{array}
 * @f]
 * where
 * @f[
 *   e_n = \sqrt{\frac{1}{L} \sum_{l=0}^{L-1} (x_n(l) - y_n(l))^2}.
 * @f]
 * If @c -f option is not specified, the average of RMSEs is calculated.
 *
 * @code{.sh}
 *   echo 1 2 3 | x2x +ad > data.1
 *   echo 0 4 3 | x2x +ad > data.2
 *   rmse data.1 data.2 -l 1 -f | x2x +da
 *   # 1 2 0
 *   rmse data.1 data.2 -l 1 | x2x +da
 *   # 1
 *   rmse data.1 data.2 | x2x +da
 *   # 1.29099
 * @endcode
 */
int main(int argc, char* argv[]) {
  int vector_length(kMagicNumberForEndOfFile);
  double magic_number(0.0);
  bool use_magic_number(kDefaultUseMagicNumber);
  bool output_frame_by_frame(kDefaultOutputFrameByFrameFlag);

  const struct option long_options[] = {
      {"magic", required_argument, NULL, kMagic},
      {0, 0, 0, 0},
  };

  for (;;) {
    const int option_char(
        getopt_long_only(argc, argv, "l:m:fh", long_options, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &vector_length) ||
            vector_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("rmse", error_message);
          return 1;
        }
        break;
      }
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &vector_length) ||
            vector_length < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("rmse", error_message);
          return 1;
        }
        ++vector_length;
        break;
      }
      case kMagic: {
        if (!sptk::ConvertStringToDouble(optarg, &magic_number)) {
          std::ostringstream error_message;
          error_message << "The argument for the -magic option must be numeric";
          sptk::PrintErrorMessage("rmse", error_message);
          return 1;
        }
        use_magic_number = true;
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

  const char* input_file1(NULL);
  const char* input_file2(NULL);
  const int num_input_files(argc - optind);
  if (2 == num_input_files) {
    input_file1 = argv[argc - 2];
    input_file2 = argv[argc - 1];
  } else if (1 == num_input_files) {
    input_file1 = argv[argc - 1];
    input_file2 = NULL;
  } else {
    std::ostringstream error_message;
    error_message << "Just two input files, file1, and infile, are required";
    sptk::PrintErrorMessage("rmse", error_message);
    return 1;
  }

  std::ifstream ifs1;
  ifs1.open(input_file1, std::ios::in | std::ios::binary);
  if (ifs1.fail()) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file1;
    sptk::PrintErrorMessage("rmse", error_message);
    return 1;
  }
  std::istream& input_stream1(ifs1);

  std::ifstream ifs2;
  ifs2.open(input_file2, std::ios::in | std::ios::binary);
  if (ifs2.fail() && NULL != input_file2) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file2;
    sptk::PrintErrorMessage("rmse", error_message);
    return 1;
  }
  std::istream& input_stream2(ifs2.fail() ? std::cin : ifs2);

  sptk::StatisticsAccumulation accumulation(0, 1);
  sptk::StatisticsAccumulation::Buffer buffer_for_mean_squared_error;
  sptk::StatisticsAccumulation::Buffer buffer_for_mean;
  if (!accumulation.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize StatisticsAccumulation";
    sptk::PrintErrorMessage("rmse", error_message);
    return 1;
  }

  const int read_size(
      kMagicNumberForEndOfFile == vector_length ? 1 : vector_length);
  std::vector<double> data1(read_size);
  std::vector<double> data2(read_size);
  while (
      sptk::ReadStream(false, 0, 0, read_size, &data1, &input_stream1, NULL) &&
      sptk::ReadStream(false, 0, 0, read_size, &data2, &input_stream2, NULL)) {
    for (int i(0); i < read_size; ++i) {
      if (!use_magic_number ||
          (magic_number != data1[i] && magic_number != data2[i])) {
        const double error(data1[i] - data2[i]);
        if (!accumulation.Run(std::vector<double>{error * error},
                              &buffer_for_mean_squared_error)) {
          std::ostringstream error_message;
          error_message << "Failed to accumulate statistics";
          sptk::PrintErrorMessage("rmse", error_message);
          return 1;
        }
      }
    }

    if (kMagicNumberForEndOfFile != vector_length) {
      std::vector<double> mean_squared_error(1);
      if (!accumulation.GetMean(buffer_for_mean_squared_error,
                                &mean_squared_error)) {
        std::ostringstream error_message;
        error_message << "Failed to get mean squared error";
        sptk::PrintErrorMessage("rmse", error_message);
        return 1;
      }

      const double root_mean_squared_error(std::sqrt(mean_squared_error[0]));
      if (output_frame_by_frame) {
        if (!sptk::WriteStream(root_mean_squared_error, &std::cout)) {
          std::ostringstream error_message;
          error_message << "Failed to write root mean squared error";
          sptk::PrintErrorMessage("rmse", error_message);
          return 1;
        }
      } else {
        if (!accumulation.Run(std::vector<double>{root_mean_squared_error},
                              &buffer_for_mean)) {
          std::ostringstream error_message;
          error_message << "Failed to accumulate statistics";
          sptk::PrintErrorMessage("rmse", error_message);
          return 1;
        }
      }
      accumulation.Clear(&buffer_for_mean_squared_error);
    }
  }

  if (kMagicNumberForEndOfFile == vector_length) {
    std::vector<double> mean_squared_error(1);
    if (!accumulation.GetMean(buffer_for_mean_squared_error,
                              &mean_squared_error)) {
      std::ostringstream error_message;
      error_message << "Failed to get mean squared error";
      sptk::PrintErrorMessage("rmse", error_message);
      return 1;
    }

    const double root_mean_squared_error(std::sqrt(mean_squared_error[0]));
    if (!sptk::WriteStream(root_mean_squared_error, &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write root mean squared error";
      sptk::PrintErrorMessage("rmse", error_message);
      return 1;
    }
  } else if (!output_frame_by_frame) {
    std::vector<double> mean(1);
    if (!accumulation.GetMean(buffer_for_mean, &mean)) {
      std::ostringstream error_message;
      error_message << "Failed to get root mean squared error";
      sptk::PrintErrorMessage("rmse", error_message);
      return 1;
    }

    if (!sptk::WriteStream(mean[0], &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write root mean squared error";
      sptk::PrintErrorMessage("rmse", error_message);
      return 1;
    }
  }

  return 0;
}
