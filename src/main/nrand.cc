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
//                1996-2017  Nagoya Institute of Technology          //
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
#include <iomanip>
#include <iostream>
#include <sstream>

#include "SPTK/generator/normal_distributed_random_value_generation.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultOutputLength(256);
const int kDefaultSeed(1);
const double kDefaultMean(0.0);
const double kDefaultStandardDeviation(1.0);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " nrand - generate normal distributed random value" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       nrand [ options ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : output length      (   int)[" << std::setw(5) << std::right << kDefaultOutputLength                                  << "][     <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : output order       (   int)[" << std::setw(5) << std::right << "l-1"                                                 << "][     <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -s s  : seed               (   int)[" << std::setw(5) << std::right << kDefaultSeed                                          << "][     <= s <=   ]" << std::endl;  // NOLINT
  *stream << "       -M M  : mean               (double)[" << std::setw(5) << std::right << kDefaultMean                                          << "][     <= M <=   ]" << std::endl;  // NOLINT
  *stream << "       -v v  : variance           (double)[" << std::setw(5) << std::right << kDefaultStandardDeviation * kDefaultStandardDeviation << "][ 0.0 <= v <=   ]" << std::endl;  // NOLINT
  *stream << "       -d d  : standard deviation (double)[" << std::setw(5) << std::right << kDefaultStandardDeviation                             << "][ 0.0 <= d <=   ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       random values              (double)" << std::endl;
  *stream << "  notice:" << std::endl;
  *stream << "       if l <= 0 or m < 0, generate infinite sequence" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

int main(int argc, char* argv[]) {
  int output_length(kDefaultOutputLength);
  int seed(kDefaultSeed);
  double mean(kDefaultMean);
  double standard_deviation(kDefaultStandardDeviation);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:s:M:v:d:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &output_length)) {
          std::ostringstream error_message;
          error_message << "The argument for the -l option must be integer";
          sptk::PrintErrorMessage("nrand", error_message);
          return 1;
        }
        break;
      }
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &output_length)) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be integer";
          sptk::PrintErrorMessage("nrand", error_message);
          return 1;
        }
        ++output_length;
        break;
      }
      case 's': {
        if (!sptk::ConvertStringToInteger(optarg, &seed)) {
          std::ostringstream error_message;
          error_message << "The argument for the -s option must be integer";
          sptk::PrintErrorMessage("nrand", error_message);
          return 1;
        }
        break;
      }
      case 'M': {
        if (!sptk::ConvertStringToDouble(optarg, &mean)) {
          std::ostringstream error_message;
          error_message << "The argument for the -M option must be double";
          sptk::PrintErrorMessage("nrand", error_message);
          return 1;
        }
        break;
      }
      case 'v': {
        double variance;
        if (!sptk::ConvertStringToDouble(optarg, &variance) || variance < 0.0) {
          std::ostringstream error_message;
          error_message << "The argument for the -v option must be double";
          sptk::PrintErrorMessage("nrand", error_message);
          return 1;
        }
        standard_deviation = std::sqrt(variance);
        break;
      }
      case 'd': {
        if (!sptk::ConvertStringToDouble(optarg, &standard_deviation) ||
            standard_deviation < 0.0) {
          std::ostringstream error_message;
          error_message << "The argument for the -d option must be double";
          sptk::PrintErrorMessage("nrand", error_message);
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
    sptk::PrintErrorMessage("nrand", error_message);
    return 1;
  }

  sptk::NormalDistributedRandomValueGeneration generator(seed);

  for (int i(0); output_length <= 0 || i < output_length; ++i) {
    double output;
    if (!generator.Get(&output)) {
      std::ostringstream error_message;
      error_message << "Failed to generate random values";
      sptk::PrintErrorMessage("nrand", error_message);
      return 1;
    }
    output = mean + output * standard_deviation;
    if (!sptk::WriteStream(output, &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write random values";
      sptk::PrintErrorMessage("nrand", error_message);
      return 1;
    }
  }

  return 0;
}
