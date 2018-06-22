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
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "SPTK/analyzer/pitch_extraction.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum LongOptions {
  kT0 = 1000,
  kT1,
  kT2,
  kT3,
};

enum OutputFormats { kPitch = 0, kF0, kLogF0, kNumOutputFormats };

const sptk::PitchExtraction::Algorithms kDefaultAlgorithm(
    sptk::PitchExtraction::Algorithms::kRapt);
const int kDefaultFrameShift(80);
const double kDefaultSamplingRate(16.0);
const double kDefaultMinimumF0(60.0);
const double kDefaultMaximumF0(240.0);
const double kDefaultVoicingThresholdForRapt(0.0);
const double kDefaultVoicingThresholdForSwipe(0.3);
const double kDefaultVoicingThresholdForReaper(0.9);
const double kDefaultVoicingThresholdForWorld(0.1);
const OutputFormats kDefaultOutputFormat(kPitch);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " pitch - pitch extraction" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       pitch [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -a a  : algorithm used for pitch      (   int)[" << std::setw(5) << std::right << kDefaultAlgorithm                 << "][    0 <= a <= 3     ]" << std::endl;  // NOLINT
  *stream << "               estimation" << std::endl;
  *stream << "                 0 (RAPT)" << std::endl;
  *stream << "                 1 (SWIPE')" << std::endl;
  *stream << "                 2 (REAPER)" << std::endl;
  *stream << "                 3 (WORLD)" << std::endl;
  *stream << "       -p p  : frame shift [point]           (   int)[" << std::setw(5) << std::right << kDefaultFrameShift                << "][    0 <  p <=       ]" << std::endl;  // NOLINT
  *stream << "       -s s  : sampling rate [kHz]           (double)[" << std::setw(5) << std::right << kDefaultSamplingRate              << "][  6.0 <  s <  98.0  ]" << std::endl;  // NOLINT
  *stream << "       -L L  : minimum fundamental frequency (double)[" << std::setw(5) << std::right << kDefaultMinimumF0                 << "][ 10.0 <  L <  H     ]" << std::endl;  // NOLINT
  *stream << "               to search for [Hz]" << std::endl;
  *stream << "       -H H  : maximum fundamental frequency (double)[" << std::setw(5) << std::right << kDefaultMaximumF0                 << "][    L <  H <  500*s ]" << std::endl;  // NOLINT
  *stream << "               to search for [Hz]" << std::endl;
  *stream << "       -t0 t : voicing threshold for RAPT    (double)[" << std::setw(5) << std::right << kDefaultVoicingThresholdForRapt   << "][ -0.6 <= t <= 0.7   ]" << std::endl;  // NOLINT
  *stream << "       -t1 t : voicing threshold for SWIPE'  (double)[" << std::setw(5) << std::right << kDefaultVoicingThresholdForSwipe  << "][  0.2 <= t <= 0.5   ]" << std::endl;  // NOLINT
  *stream << "       -t2 t : voicing threshold for REAPER  (double)[" << std::setw(5) << std::right << kDefaultVoicingThresholdForReaper << "][ -0.5 <= t <= 1.6   ]" << std::endl;  // NOLINT
  *stream << "       -t3 t : voicing threshold for WORLD   (double)[" << std::setw(5) << std::right << kDefaultVoicingThresholdForWorld  << "][ 0.02 <= t <= 0.2   ]" << std::endl;  // NOLINT
  *stream << "       -o o  : output format                 (   int)[" << std::setw(5) << std::right << kDefaultOutputFormat              << "][    0 <= o <= 2     ]" << std::endl;  // NOLINT
  *stream << "                 0 (pitch)" << std::endl;
  *stream << "                 1 (F0)" << std::endl;
  *stream << "                 2 (log F0)" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       waveform                              (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       pitch                                 (double)" << std::endl;  // NOLINT
  *stream << "  notice:" << std::endl;
  *stream << "       if t is raised, the number of voiced frames will increase in RAPT, REAPER, and WORLD" << std::endl;  // NOLINT
  *stream << "       if t is dropped, the number of voiced frames will increase in SWIPE'" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

int main(int argc, char* argv[]) {
  sptk::PitchExtraction::Algorithms algorithm(kDefaultAlgorithm);
  int frame_shift(kDefaultFrameShift);
  double sampling_rate(kDefaultSamplingRate);
  double minimum_f0(kDefaultMinimumF0);
  double maximum_f0(kDefaultMaximumF0);
  std::vector<double> voicing_thresholds{
      kDefaultVoicingThresholdForRapt, kDefaultVoicingThresholdForSwipe,
      kDefaultVoicingThresholdForReaper, kDefaultVoicingThresholdForWorld,
  };
  OutputFormats output_format(kDefaultOutputFormat);

  const struct option long_options[] = {
      {"t0", required_argument, NULL, kT0},
      {"t1", required_argument, NULL, kT1},
      {"t2", required_argument, NULL, kT2},
      {"t3", required_argument, NULL, kT3},
      {0, 0, 0, 0},
  };

  for (;;) {
    const int option_char(
        getopt_long_only(argc, argv, "a:p:s:L:H:o:h", long_options, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'a': {
        const int min(0);
        const int max(static_cast<int>(
                          sptk::PitchExtraction::Algorithms::kNumAlgorithms) -
                      1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -a option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("pitch", error_message);
          return 1;
        }
        algorithm = static_cast<sptk::PitchExtraction::Algorithms>(tmp);
        break;
      }
      case 'p': {
        if (!sptk::ConvertStringToInteger(optarg, &frame_shift) ||
            frame_shift <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -p option must be a positive integer";
          sptk::PrintErrorMessage("pitch", error_message);
          return 1;
        }
        break;
      }
      case 's': {
        const double min(6.0);
        const double max(98.0);
        if (!sptk::ConvertStringToDouble(optarg, &sampling_rate) ||
            sampling_rate <= min || max < sampling_rate) {
          std::ostringstream error_message;
          error_message << "The argument for the -s option must be a number "
                        << "in the interval (" << min << ", " << max << "]";
          sptk::PrintErrorMessage("pitch", error_message);
          return 1;
        }
        break;
      }
      case 'L': {
        if (!sptk::ConvertStringToDouble(optarg, &minimum_f0) ||
            minimum_f0 <= 10.0) {
          std::ostringstream error_message;
          error_message << "The argument for the -L option must be a number "
                        << "greater than 10";
          sptk::PrintErrorMessage("pitch", error_message);
          return 1;
        }
        break;
      }
      case 'H': {
        if (!sptk::ConvertStringToDouble(optarg, &maximum_f0) ||
            maximum_f0 <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -H option must be a positive number";
          sptk::PrintErrorMessage("pitch", error_message);
          return 1;
        }
        break;
      }
      case kT0: {
        const double min(-0.6);
        const double max(0.7);
        double tmp;
        if (!sptk::ConvertStringToDouble(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -t0 option must be a number "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("pitch", error_message);
          return 1;
        }
        voicing_thresholds[sptk::PitchExtraction::Algorithms::kRapt] = tmp;
        break;
      }
      case kT1: {
        const double min(0.2);
        const double max(0.5);
        double tmp;
        if (!sptk::ConvertStringToDouble(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -t1 option must be a number "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("pitch", error_message);
          return 1;
        }
        voicing_thresholds[sptk::PitchExtraction::Algorithms::kSwipe] = tmp;
        break;
      }
      case kT2: {
        const double min(-0.5);
        const double max(1.6);
        double tmp;
        if (!sptk::ConvertStringToDouble(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -t2 option must be a number "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("pitch", error_message);
          return 1;
        }
        voicing_thresholds[sptk::PitchExtraction::Algorithms::kReaper] = tmp;
        break;
      }
      case kT3: {
        const double min(0.02);
        const double max(0.2);
        double tmp;
        if (!sptk::ConvertStringToDouble(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -t3 option must be a number "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("pitch", error_message);
          return 1;
        }
        voicing_thresholds[sptk::PitchExtraction::Algorithms::kWorld] = tmp;
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
          sptk::PrintErrorMessage("pitch", error_message);
          return 1;
        }
        output_format = static_cast<OutputFormats>(tmp);
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

  const double sampling_rate_in_hz(1000.0 * sampling_rate);
  if (0.5 * sampling_rate_in_hz <= maximum_f0) {
    std::ostringstream error_message;
    error_message
        << "Maximum fundamental frequency must be less than Nyquist frequency";
    sptk::PrintErrorMessage("pitch", error_message);
    return 1;
  }

  if (maximum_f0 <= minimum_f0) {
    std::ostringstream error_message;
    error_message
        << "Minimum fundamental frequency must be less than maximum one";
    sptk::PrintErrorMessage("pitch", error_message);
    return 1;
  }

  // get input file
  const int num_rest_args(argc - optind);
  if (1 < num_rest_args) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("pitch", error_message);
    return 1;
  }
  const char* input_file(0 == num_rest_args ? NULL : argv[optind]);

  // open stream
  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("pitch", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  // prepare for pitch extraction
  sptk::PitchExtraction pitch_extraction(
      frame_shift, sampling_rate_in_hz, minimum_f0, maximum_f0,
      voicing_thresholds[algorithm], algorithm);
  if (!pitch_extraction.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to set condition for pitch extraction";
    sptk::PrintErrorMessage("pitch", error_message);
    return 1;
  }

  std::vector<double> waveform;
  {
    double tmp;
    while (sptk::ReadStream(&tmp, &input_stream)) {
      waveform.push_back(tmp);
    }
  }
  if (waveform.empty()) return 0;

  std::vector<double> f0;
  if (!pitch_extraction.Run(waveform, &f0, NULL, NULL)) {
    std::ostringstream error_message;
    error_message << "Failed to extract pitch";
    sptk::PrintErrorMessage("pitch", error_message);
    return 1;
  }

  switch (output_format) {
    case kPitch: {
      std::transform(f0.begin(), f0.end(), f0.begin(),
                     [sampling_rate_in_hz](double x) {
                       return (0.0 < x) ? sampling_rate_in_hz / x : 0.0;
                     });
      break;
    }
    case kF0: {
      // nothing to do
      break;
    }
    case kLogF0: {
      std::transform(f0.begin(), f0.end(), f0.begin(),
                     [sampling_rate_in_hz](double x) {
                       return (0.0 < x) ? std::log(x) : sptk::kLogZero;
                     });
      break;
    }
    default: { break; }
  }

  if (!sptk::WriteStream(0, f0.size(), f0, &std::cout, NULL)) {
    std::ostringstream error_message;
    error_message << "Failed to write pitch";
    sptk::PrintErrorMessage("pitch", error_message);
    return 1;
  }

  return 0;
}
