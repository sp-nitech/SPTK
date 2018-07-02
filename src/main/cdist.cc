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

#include "SPTK/math/distance_calculator.h"
#include "SPTK/math/statistics_accumulator.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum OutputFormats {
  kEuclideanInDecibel = 0,
  kEuclidean,
  kSquaredEuclidean,
  kNumOutputFormats
};

const int kDefaultNumOrder(25);
const OutputFormats kDefaultOutputFormat(kEuclideanInDecibel);
const bool kDefaultOutputFrameByFrameFlag(false);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " cdist - calculate cepstral distance" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       cdist [ options ] cfile [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -m m  : order of cepstrum     (   int)[" << std::setw(5) << std::right << kDefaultNumOrder     << "][ 1 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -o o  : output format         (   int)[" << std::setw(5) << std::right << kDefaultOutputFormat << "][ 0 <= o <= 2 ]" << std::endl;  // NOLINT
  *stream << "                 0 (euclidean distance [dB])" << std::endl;
  *stream << "                 1 (euclidean distance)" << std::endl;
  *stream << "                 2 (squared euclidean distance)" << std::endl;
  *stream << "       -f    : output frame by frame (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultOutputFrameByFrameFlag) << "]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  cfile:" << std::endl;
  *stream << "       minimum-phase cepstrum        (double)" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       minimum-phase cepstrum        (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       cepstral distance             (double)" << std::endl;
  *stream << "  notice:" << std::endl;
  *stream << "       0th cepstral coefficients are ignored" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);
  OutputFormats output_format(kDefaultOutputFormat);
  bool output_frame_by_frame(kDefaultOutputFrameByFrameFlag);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "m:o:fh", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -m option must be a positive integer";
          sptk::PrintErrorMessage("cdist", error_message);
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
          sptk::PrintErrorMessage("cdist", error_message);
          return 1;
        }
        output_format = static_cast<OutputFormats>(tmp);
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

  const char* cepstrum1_file;
  const char* cepstrum2_file;
  const int num_rest_args(argc - optind);
  if (2 == num_rest_args) {
    cepstrum1_file = argv[argc - 2];
    cepstrum2_file = argv[argc - 1];
  } else if (1 == num_rest_args) {
    cepstrum1_file = argv[argc - 1];
    cepstrum2_file = NULL;
  } else {
    std::ostringstream error_message;
    error_message << "Just two input files, cfile and infile, are required";
    sptk::PrintErrorMessage("cdist", error_message);
    return 1;
  }

  std::ifstream ifs1;
  ifs1.open(cepstrum1_file, std::ios::in | std::ios::binary);
  if (ifs1.fail()) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << cepstrum1_file;
    sptk::PrintErrorMessage("cdist", error_message);
    return 1;
  }
  std::istream& stream_for_cepstrum1(ifs1);

  std::ifstream ifs2;
  ifs2.open(cepstrum2_file, std::ios::in | std::ios::binary);
  if (ifs2.fail() && NULL != cepstrum2_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << cepstrum2_file;
    sptk::PrintErrorMessage("cdist", error_message);
    return 1;
  }
  std::istream& stream_for_cepstrum2(ifs2.fail() ? std::cin : ifs2);

  sptk::StatisticsAccumulator statistics_accumulator(0, 1);
  sptk::StatisticsAccumulator::Buffer buffer;
  sptk::DistanceCalculator distance_calculator(
      num_order - 1,
      sptk::DistanceCalculator::DistanceMetrics::kSquaredEuclidean);
  if (!statistics_accumulator.IsValid() || !distance_calculator.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to set condition for calculation";
    sptk::PrintErrorMessage("cdist", error_message);
    return 1;
  }

  std::vector<double> cepstrum1(num_order);
  std::vector<double> cepstrum2(num_order);

  while (sptk::ReadStream(false, 1, 0, num_order, &cepstrum1,
                          &stream_for_cepstrum1, NULL) &&
         sptk::ReadStream(false, 1, 0, num_order, &cepstrum2,
                          &stream_for_cepstrum2, NULL)) {
    double distance;
    if (!distance_calculator.Run(cepstrum1, cepstrum2, &distance)) {
      std::ostringstream error_message;
      error_message << "Failed to calculate distance";
      sptk::PrintErrorMessage("cdist", error_message);
      return 1;
    }

    switch (output_format) {
      case kEuclideanInDecibel: {
        distance = 0.5 * sptk::kNeper * std::sqrt(2.0 * distance);
        break;
      }
      case kEuclidean: {
        distance = std::sqrt(distance);
        break;
      }
      case kSquaredEuclidean: {
        // nothing to do
        break;
      }
      default: { break; }
    }

    if (output_frame_by_frame) {
      if (!sptk::WriteStream(distance, &std::cout)) {
        std::ostringstream error_message;
        error_message << "Failed to write distance";
        sptk::PrintErrorMessage("cdist", error_message);
        return 1;
      }
    } else {
      if (!statistics_accumulator.Run(std::vector<double>{distance}, &buffer)) {
        std::ostringstream error_message;
        error_message << "Failed to accumulate statistics";
        sptk::PrintErrorMessage("cdist", error_message);
        return 1;
      }
    }
  }

  int num_data;
  if (!statistics_accumulator.GetNumData(buffer, &num_data)) {
    std::ostringstream error_message;
    error_message << "Failed to accumulate statistics";
    sptk::PrintErrorMessage("cdist", error_message);
    return 1;
  }

  if (!output_frame_by_frame && 0 < num_data) {
    std::vector<double> average_distance(1);
    if (!statistics_accumulator.GetMean(buffer, &average_distance)) {
      std::ostringstream error_message;
      error_message << "Failed to calculate distance";
      sptk::PrintErrorMessage("average", error_message);
      return 1;
    }

    if (!sptk::WriteStream(0, 1, average_distance, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write distance";
      sptk::PrintErrorMessage("cdist", error_message);
      return 1;
    }
  }

  return 0;
}
