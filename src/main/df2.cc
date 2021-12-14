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

#include <algorithm>  // std::max
#include <exception>  // std::exception
#include <fstream>    // std::ifstream
#include <iomanip>    // std::setw
#include <iostream>   // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>    // std::ostringstream
#include <vector>     // std::vector

#include "Getopt/getoptwin.h"
#include "SPTK/filter/second_order_digital_filter.h"
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
  *stream << "       -s s     : sampling rate [kHz]               (double)[" << std::setw(5) << std::right << kDefaultSamplingRate << "][ 0.0 <  s  <=       ]" << std::endl;  // NOLINT
  *stream << "       -p f1 b1 : pole frequency and bandwidth [Hz] (double)[" << std::setw(5) << std::right << "N/A"                << "][ 0.0 <  f1 <  500*s ][ 0.0 <  b1 <=   ]" << std::endl;  // NOLINT
  *stream << "       -z f2 b2 : zero frequency and bandwidth [Hz] (double)[" << std::setw(5) << std::right << "N/A"                << "][ 0.0 <  f2 <  500*s ][ 0.0 <  b2 <=   ]" << std::endl;  // NOLINT
  *stream << "       -h       : print this message" << std::endl;
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

/**
 * @a df2 [ @e option ] [ @e infile ]
 *
 * - @b -s @e double
 *   - sampling rate in kHz @f$(0 < F_s)@f$
 * - @b -p @e double @e double
 *   - pole frequency and bandwidth in Hz @f$(0 < F_1 < 500F_s, \, 0 < B_1)@f$
 * - @b -z @e double @e double
 *   - zero frequency and bandwidth in Hz @f$(0 < F_2 < 500F_s, \, 0 < B_2)@f$
 * - @b infile @e str
 *   - double-type filter input
 * - @b stdout
 *   - double-type filter output
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
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

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("df2", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("df2", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  const int num_pole_filter(static_cast<int>(pole_frequencies.size()));
  const int num_zero_filter(static_cast<int>(zero_frequencies.size()));
  const int num_filter(std::max(num_pole_filter, num_zero_filter));
  std::vector<sptk::SecondOrderDigitalFilter*> filters;
  std::vector<sptk::SecondOrderDigitalFilter::Buffer> buffers(num_filter);

  if (0 == num_filter) {
    std::ostringstream error_message;
    error_message << "One or more -p or -z options are required";
    sptk::PrintErrorMessage("df2", error_message);
    return 1;
  }

  try {
    for (int i(0); i < num_filter; ++i) {
      if (i < num_pole_filter && i < num_zero_filter) {
        filters.push_back(new sptk::SecondOrderDigitalFilter(
            pole_frequencies[i], pole_bandwidths[i], zero_frequencies[i],
            zero_bandwidths[i], sampling_rate_in_hz));
      } else if (i < num_pole_filter) {
        filters.push_back(new sptk::SecondOrderDigitalFilter(
            sptk::SecondOrderDigitalFilter::kPole, pole_frequencies[i],
            pole_bandwidths[i], sampling_rate_in_hz));
      } else if (i < num_zero_filter) {
        filters.push_back(new sptk::SecondOrderDigitalFilter(
            sptk::SecondOrderDigitalFilter::kZero, zero_frequencies[i],
            zero_bandwidths[i], sampling_rate_in_hz));
      }
      if (!filters.back()->IsValid()) {
        std::ostringstream error_message;
        error_message << "Failed to initialize SecondOrderDigitalFilter";
        sptk::PrintErrorMessage("df2", error_message);
        for (sptk::SecondOrderDigitalFilter* filter : filters) {
          delete filter;
        }
        return 1;
      }
    }

    double signal;
    while (sptk::ReadStream(&signal, &input_stream)) {
      for (int i(0); i < num_filter; ++i) {
        if (!filters[i]->Run(&signal, &buffers[i])) {
          std::ostringstream error_message;
          error_message << "Failed to apply second order digital filter";
          sptk::PrintErrorMessage("df2", error_message);
          for (sptk::SecondOrderDigitalFilter* filter : filters) {
            delete filter;
          }
          return 1;
        }
      }

      if (!sptk::WriteStream(signal, &std::cout)) {
        std::ostringstream error_message;
        error_message << "Failed to write a filter output";
        sptk::PrintErrorMessage("df2", error_message);
        for (sptk::SecondOrderDigitalFilter* filter : filters) {
          delete filter;
        }
        return 1;
      }
    }
  } catch (std::exception&) {
    std::ostringstream error_message;
    error_message << "Unknown exception";
    sptk::PrintErrorMessage("df2", error_message);
    for (sptk::SecondOrderDigitalFilter* filter : filters) {
      delete filter;
    }
    return 1;
  }

  for (sptk::SecondOrderDigitalFilter* filter : filters) {
    delete filter;
  }

  return 0;
}
