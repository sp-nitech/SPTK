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
//                1996-2019  Nagoya Institute of Technology          //
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

#include <getopt.h>  // getopt_long
#include <cmath>     // std::log10
#include <fstream>   // std::ifstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream
#include <vector>    // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace {

enum OutputType {
  kSnr = 0,
  kSegmentalSnr,
  kSegmentalSnrPerFrame,
  kNumOutputTypes
};

const int kDefaultFrameLength(256);
const OutputType kDefaultOutputType(OutputType::kSnr);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " snr - evaluate SNR and segmental SNR" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       snr [ options ] file1 [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : frame length       (   int)[" << std::setw(5) << std::right << kDefaultFrameLength << "][ 0 <  l <=   ]" << std::endl;  // NOLINT
  *stream << "       -o o  : output type        (   int)[" << std::setw(5) << std::right << kDefaultOutputType  << "][ 0 <= o <= 2 ]" << std::endl;  // NOLINT
  *stream << "                 0 (SNR)" << std::endl;
  *stream << "                 1 (segmental SNR)" << std::endl;
  *stream << "                 2 (segmental SNR per frame)" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  file1:" << std::endl;
  *stream << "       signal sequence            (double)" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       signal plus noise sequence (double)[stdin]" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       SNR                        (double)" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

int main(int argc, char* argv[]) {
  int frame_length(kDefaultFrameLength);
  OutputType output_type(kDefaultOutputType);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:o:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &frame_length) ||
            frame_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("snr", error_message);
          return 1;
        }
        break;
      }
      case 'o': {
        const int min(0);
        const int max(static_cast<int>(OutputType::kNumOutputTypes) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -o option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("snr", error_message);
          return 1;
        }
        output_type = static_cast<OutputType>(tmp);
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

  // Get input file names.
  const char* signal_file;
  const char* signal_plus_noise_file;
  const int num_input_files(argc - optind);
  if (2 == num_input_files) {
    signal_file = argv[argc - 2];
    signal_plus_noise_file = argv[argc - 1];
  } else if (1 == num_input_files) {
    signal_file = argv[argc - 1];
    signal_plus_noise_file = NULL;
  } else {
    std::ostringstream error_message;
    error_message << "Just two input files, file1 and infile, are required";
    sptk::PrintErrorMessage("snr", error_message);
    return 1;
  }

  // Open stream for reading signal sequence.
  std::ifstream ifs1;
  ifs1.open(signal_file, std::ios::in | std::ios::binary);
  if (ifs1.fail()) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << signal_file;
    sptk::PrintErrorMessage("snr", error_message);
    return 1;
  }
  std::istream& stream_for_signal(ifs1);

  // Open stream for reading signal plus noise sequence.
  std::ifstream ifs2;
  ifs2.open(signal_plus_noise_file, std::ios::in | std::ios::binary);
  if (ifs2.fail() && NULL != signal_plus_noise_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << signal_plus_noise_file;
    sptk::PrintErrorMessage("snr", error_message);
    return 1;
  }
  std::istream& stream_for_signal_plus_noise(ifs2.fail() ? std::cin : ifs2);

  if (OutputType::kSnr == output_type) {
    double signal;
    double signal_plus_noise;
    double signal_power(0.0);
    double noise_power(0.0);
    while (
        sptk::ReadStream(&signal, &stream_for_signal) &&
        sptk::ReadStream(&signal_plus_noise, &stream_for_signal_plus_noise)) {
      signal_power += signal * signal;

      const double noise(signal_plus_noise - signal);
      noise_power += noise * noise;
    }

    if (0.0 == signal_power) {
      std::ostringstream error_message;
      error_message << "The signal power is 0.0";
      sptk::PrintErrorMessage("snr", error_message);
      return 1;
    }
    if (0.0 == noise_power) {
      std::ostringstream error_message;
      error_message << "The noise power is 0.0";
      sptk::PrintErrorMessage("snr", error_message);
      return 1;
    }

    const double snr(10.0 * std::log10(signal_power / noise_power));
    if (!sptk::WriteStream(snr, &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write SNR";
      sptk::PrintErrorMessage("snr", error_message);
      return 1;
    }
  } else if (OutputType::kSegmentalSnr == output_type ||
             OutputType::kSegmentalSnrPerFrame == output_type) {
    std::vector<double> signal(frame_length);
    std::vector<double> signal_plus_noise(frame_length);
    double segmental_snr(0.0);
    int frame_index(0);
    for (; sptk::ReadStream(false, 0, 0, frame_length, &signal,
                            &stream_for_signal, NULL) &&
           sptk::ReadStream(false, 0, 0, frame_length, &signal_plus_noise,
                            &stream_for_signal_plus_noise, NULL);
         ++frame_index) {
      double segmental_signal_power(0.0);
      double segmental_noise_power(0.0);

      for (int i(0); i < frame_length; ++i) {
        segmental_signal_power += signal[i] * signal[i];

        const double noise(signal_plus_noise[i] - signal[i]);
        segmental_noise_power += noise * noise;
      }
      if (0.0 == segmental_signal_power) {
        std::ostringstream error_message;
        error_message << "The signal power of " << frame_index
                      << "th frame is 0.0";
        sptk::PrintErrorMessage("snr", error_message);
        return 1;
      }
      if (0.0 == segmental_noise_power) {
        std::ostringstream error_message;
        error_message << "The noise power of " << frame_index
                      << "th frame is 0.0";
        sptk::PrintErrorMessage("snr", error_message);
        return 1;
      }

      const double segmental_snr_of_current_frame(
          10.0 * std::log10(segmental_signal_power / segmental_noise_power));
      if (OutputType::kSegmentalSnr == output_type) {
        segmental_snr += segmental_snr_of_current_frame;
      } else if (OutputType::kSegmentalSnrPerFrame == output_type) {
        if (!sptk::WriteStream(segmental_snr_of_current_frame, &std::cout)) {
          std::ostringstream error_message;
          error_message << "Failed to write segmental SNR of " << frame_index
                        << "th frame";
          sptk::PrintErrorMessage("snr", error_message);
          return 1;
        }
      }
    }

    if (OutputType::kSegmentalSnr == output_type && 0 < frame_index) {
      segmental_snr /= frame_index;
      if (!sptk::WriteStream(segmental_snr, &std::cout)) {
        std::ostringstream error_message;
        error_message << "Failed to write segmental SNR";
        sptk::PrintErrorMessage("snr", error_message);
        return 1;
      }
    }
  }

  return 0;
}
