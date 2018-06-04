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

#include <getopt.h>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "SPTK/math/statistics_accumulator.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kMagic(1000);
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
  *stream << "       -l l         : length of vector      (   int)[" << std::setw(5) << std::right << "EOF" << "][ 1 <=   l   <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m         : order of vector       (   int)[" << std::setw(5) << std::right << "l-1" << "][ 0 <=   m   <=   ]" << std::endl;  // NOLINT
  *stream << "       -magic magic : remove magic number   (double)[" << std::setw(5) << std::right << "N/A" << "][   <= magic <=   ]" << std::endl;  // NOLINT
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

int main(int argc, char* argv[]) {
  int vector_length(kMagicNumberForEndOfFile);
  double magic_number(0.0);
  bool use_magic_number(kDefaultUseMagicNumber);
  bool output_frame_by_frame(kDefaultOutputFrameByFrameFlag);

  const struct option long_option[] = {
      {"magic", required_argument, NULL, kMagic}, {0, 0, 0, 0},
  };

  for (;;) {
    const int option_char(
        getopt_long_only(argc, argv, "l:m:fh", long_option, NULL));
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

  // get input file
  const char* input_file1(NULL);
  const char* input_file2(NULL);
  const int num_rest_args(argc - optind);
  if (2 == num_rest_args) {
    input_file1 = argv[argc - 2];
    input_file2 = argv[argc - 1];
  } else if (1 == num_rest_args) {
    input_file1 = argv[argc - 1];
    input_file2 = NULL;
  } else {
    std::ostringstream error_message;
    error_message << "Just two input files, file1, and infile, are required";
    sptk::PrintErrorMessage("rmse", error_message);
    return 1;
  }

  // open stream
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

  sptk::StatisticsAccumulator accumulator(0, 1);
  sptk::StatisticsAccumulator::Buffer buffer_for_mean_squared_error;
  sptk::StatisticsAccumulator::Buffer buffer_for_mean;
  if (!accumulator.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to set condition for calculation";
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
        if (!accumulator.Run(std::vector<double>{error * error},
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
      if (!accumulator.GetMean(buffer_for_mean_squared_error,
                               &mean_squared_error)) {
        std::ostringstream error_message;
        error_message << "Failed to accumulate statistics";
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
        if (!accumulator.Run(std::vector<double>{root_mean_squared_error},
                             &buffer_for_mean)) {
          std::ostringstream error_message;
          error_message << "Failed to accumulate statistics";
          sptk::PrintErrorMessage("rmse", error_message);
          return 1;
        }
      }
      accumulator.Clear(&buffer_for_mean_squared_error);
    }
  }

  if (kMagicNumberForEndOfFile == vector_length) {
    std::vector<double> mean_squared_error(1);
    if (!accumulator.GetMean(buffer_for_mean_squared_error,
                             &mean_squared_error)) {
      std::ostringstream error_message;
      error_message << "Failed to accumulate statistics";
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
    if (!accumulator.GetMean(buffer_for_mean, &mean)) {
      std::ostringstream error_message;
      error_message << "Failed to accumulate statistics";
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
