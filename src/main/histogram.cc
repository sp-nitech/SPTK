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

#include <getopt.h>    // getopt_long
#include <algorithm>   // std::transform
#include <fstream>     // std::ifstream
#include <functional>  // std::bind1st, std::multiplies
#include <iomanip>     // std::setw
#include <iostream>    // std::cerr, std::cin, std::cout, std::endl, etc.
#include <numeric>     // std::accumulate
#include <sstream>     // std::ostringstream
#include <vector>      // std::vector

#include "SPTK/math/histogram_calculator.h"
#include "SPTK/math/statistics_accumulator.h"
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

  // get input file
  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("histogram", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  // open stream
  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("histogram", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  // prepare for calculating histogram
  const int data_length(
      kMagicNumberForEndOfFile == output_interval ? 1 : output_interval);
  sptk::HistogramCalculator histogram_calculator(data_length, num_bin,
                                                 lower_bound, upper_bound);
  if (!histogram_calculator.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to set condition for calculating histogram";
    sptk::PrintErrorMessage("histogram", error_message);
    return 1;
  }

  std::vector<double> data(data_length);
  std::vector<double> histogram(num_bin);

  if (kMagicNumberForEndOfFile == output_interval) {
    sptk::StatisticsAccumulator statistics_accumulator(num_bin - 1, 1);
    sptk::StatisticsAccumulator::Buffer buffer;
    while (sptk::ReadStream(false, 0, 0, 1, &data, &input_stream, NULL)) {
      if (!histogram_calculator.Run(data, &histogram)) {
        std::ostringstream error_message;
        error_message << "Failed to calculate histogram";
        sptk::PrintErrorMessage("histogram", error_message);
        return 1;
      }
      if (!statistics_accumulator.Run(histogram, &buffer)) {
        std::ostringstream error_message;
        error_message << "Failed to accumulate histogram";
        sptk::PrintErrorMessage("histogram", error_message);
        return 1;
      }
    }

    if (!statistics_accumulator.GetSum(buffer, &histogram)) {
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
        error_message << "Cannot calculate normalized histogram";
        sptk::PrintErrorMessage("histogram", error_message);
        return 1;
      }
      std::transform(histogram.begin(), histogram.end(), histogram.begin(),
                     std::bind1st(std::multiplies<double>(), 1.0 / sum));
    }

    if (!sptk::WriteStream(0, num_bin, histogram, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write histogram";
      sptk::PrintErrorMessage("histogram", error_message);
      return 1;
    }
  } else {
    for (int frame_index(0);
         sptk::ReadStream(false, 0, 0, data_length, &data, &input_stream, NULL);
         ++frame_index) {
      if (!histogram_calculator.Run(data, &histogram)) {
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
          error_message << "Cannot calculate normalized histogram at "
                        << frame_index << "th frame";
          sptk::PrintErrorMessage("histogram", error_message);
          return 1;
        }
        std::transform(histogram.begin(), histogram.end(), histogram.begin(),
                       std::bind1st(std::multiplies<double>(), 1.0 / sum));
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
