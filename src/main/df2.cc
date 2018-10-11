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

#include <getopt.h>   // getopt_long
#include <algorithm>  // std::max
#include <cmath>      // std::cos, std::exp
#include <fstream>    // std::ifstream
#include <iomanip>    // std::setw
#include <iostream>   // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>    // std::ostringstream
#include <vector>     // std::vector

#include "SPTK/filter/infinite_impulse_response_digital_filter.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const double kDefaultSamplingRate(10.0);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " df2 - second order digital filter" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       df2 [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -s s   : sampling rate [kHz]               (double)[" << std::setw(5) << std::right << kDefaultSamplingRate << "][ 0.0 <  s <=       ]" << std::endl;  // NOLINT
  *stream << "       -p f b : pole frequency and bandwidth [Hz] (double)[" << std::setw(5) << std::right << "N/A"                << "][ 0.0 <  f <  500*s ][ 0.0 <  b <=   ]" << std::endl;  // NOLINT
  *stream << "       -z q w : zero frequency and bandwidth [Hz] (double)[" << std::setw(5) << std::right << "N/A"                << "][ 0.0 <  q <  500*s ][ 0.0 <  w <=   ]" << std::endl;  // NOLINT
  *stream << "       -h     : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       filter input                               (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       filter output                              (double)" << std::endl;  // NOLINT
  *stream << "  notice:" << std::endl;
  *stream << "       -p and -z options can be specified multiple times" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

int main(int argc, char* argv[]) {
  double sampling_rate(kDefaultSamplingRate);
  std::vector<double> pole_frequencies;
  std::vector<double> pole_bandwidths;
  std::vector<double> zero_frequencies;
  std::vector<double> zero_bandwidths;

  for (;;) {
    const int option_char(getopt_long(argc, argv, "s:p:z:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 's': {
        if (!sptk::ConvertStringToDouble(optarg, &sampling_rate) ||
            sampling_rate <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -s option must be a positive number";
          sptk::PrintErrorMessage("df2", error_message);
          return 1;
        }
        break;
      }
      case 'p': {
        double pole_frequency;
        if (!sptk::ConvertStringToDouble(optarg, &pole_frequency) ||
            pole_frequency <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The 1st argument for the -p option must be a positive number";
          sptk::PrintErrorMessage("df2", error_message);
          return 1;
        }
        double pole_bandwidth;
        if (argc <= optind ||
            !sptk::ConvertStringToDouble(argv[optind], &pole_bandwidth) ||
            pole_bandwidth <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The 2nd argument for the -p option must be a positive number";
          sptk::PrintErrorMessage("df2", error_message);
          return 1;
        }
        ++optind;
        pole_frequencies.push_back(pole_frequency);
        pole_bandwidths.push_back(pole_bandwidth);
        break;
      }
      case 'z': {
        double zero_frequency;
        if (!sptk::ConvertStringToDouble(optarg, &zero_frequency) ||
            zero_frequency <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The 1st argument for the -z option must be a positive number";
          sptk::PrintErrorMessage("df2", error_message);
          return 1;
        }
        double zero_bandwidth;
        if (argc <= optind ||
            !sptk::ConvertStringToDouble(argv[optind], &zero_bandwidth) ||
            zero_bandwidth <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The 2nd argument for the -z option must be a positive number";
          sptk::PrintErrorMessage("df2", error_message);
          return 1;
        }
        ++optind;
        zero_frequencies.push_back(zero_frequency);
        zero_bandwidths.push_back(zero_bandwidth);
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
  const double nyquist_frequency(0.5 * sampling_rate_in_hz);
  for (double pole_frequency : pole_frequencies) {
    if (nyquist_frequency <= pole_frequency) {
      std::ostringstream error_message;
      error_message << "Pole frequency must be less than Nyquist frequency";
      sptk::PrintErrorMessage("df2", error_message);
      return 1;
    }
  }
  for (double zero_frequency : zero_frequencies) {
    if (nyquist_frequency <= zero_frequency) {
      std::ostringstream error_message;
      error_message << "Zero frequency must be less than Nyquist frequency";
      sptk::PrintErrorMessage("df2", error_message);
      return 1;
    }
  }

  // get input file
  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("df2", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  // open stream
  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("df2", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  const int num_pole_filter(pole_frequencies.size());
  const int num_zero_filter(zero_frequencies.size());
  const int num_filter(std::max(std::max(num_pole_filter, num_zero_filter), 1));
  std::vector<sptk::InfiniteImpulseResponseDigitalFilter*> filters;
  std::vector<sptk::InfiniteImpulseResponseDigitalFilter::Buffer> buffers(
      num_filter);

  try {
    for (int i(0); i < num_filter; ++i) {
      std::vector<double> denominator_coefficients(3);
      denominator_coefficients[0] = 1.0;
      if (i < num_pole_filter) {
        const double pole_radius(
            std::exp(-sptk::kPi * pole_bandwidths[i] / sampling_rate_in_hz));
        denominator_coefficients[1] =
            -2.0 * pole_radius *
            std::cos(2.0 * sptk::kPi * pole_frequencies[i] /
                     sampling_rate_in_hz);
        denominator_coefficients[2] = pole_radius * pole_radius;
      }

      std::vector<double> numerator_coefficients(3);
      numerator_coefficients[0] = 1.0;
      if (i < num_zero_filter) {
        const double zero_radius(
            std::exp(-sptk::kPi * zero_bandwidths[i] / sampling_rate_in_hz));
        numerator_coefficients[1] =
            -2.0 * zero_radius *
            std::cos(2.0 * sptk::kPi * zero_frequencies[i] /
                     sampling_rate_in_hz);
        numerator_coefficients[2] = zero_radius * zero_radius;
      }

      filters.push_back(new sptk::InfiniteImpulseResponseDigitalFilter(
          denominator_coefficients, numerator_coefficients));

      if (!filters.back()->IsValid()) {
        std::ostringstream error_message;
        error_message << "Failed to set condition for filtering";
        sptk::PrintErrorMessage("df2", error_message);
        throw;
      }
    }

    double filter_input, filter_output;
    while (sptk::ReadStream(&filter_input, &input_stream)) {
      for (int i(0); i < num_filter; ++i) {
        if (!filters[i]->Run(filter_input, &filter_output, &buffers[i])) {
          std::ostringstream error_message;
          error_message << "Failed to apply digital filter";
          sptk::PrintErrorMessage("df2", error_message);
          throw;
        }
        filter_input = filter_output;
      }

      if (!sptk::WriteStream(filter_output, &std::cout)) {
        std::ostringstream error_message;
        error_message << "Failed to write a filter output";
        sptk::PrintErrorMessage("df2", error_message);
        throw;
      }
    }
  } catch (...) {
    for (sptk::InfiniteImpulseResponseDigitalFilter* filter : filters) {
      delete filter;
    }

    return 1;
  }

  for (sptk::InfiniteImpulseResponseDigitalFilter* filter : filters) {
    delete filter;
  }

  return 0;
}
