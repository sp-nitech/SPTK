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
#include "SPTK/filter/all_pole_lattice_digital_filter.h"
#include "SPTK/input/input_source_from_stream.h"
#include "SPTK/input/input_source_interpolation.h"
#include "SPTK/input/input_source_preprocessing_for_filter_gain.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultNumFilterOrder(25);
const int kDefaultFramePeriod(100);
const int kDefaultInterpolationPeriod(1);
const bool kDefaultGainFlag(true);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " ltcdf - all-pole lattice digital filter for speech synthesis" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       ltcdf [ options ] kfile [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -m m  : order of filter coefficients (   int)[" << std::setw(5) << std::right << kDefaultNumFilterOrder      << "][ 0 <= m <=     ]" << std::endl;  // NOLINT
  *stream << "       -p p  : frame period                 (   int)[" << std::setw(5) << std::right << kDefaultFramePeriod         << "][ 0 <  p <=     ]" << std::endl;  // NOLINT
  *stream << "       -i i  : interpolation period         (   int)[" << std::setw(5) << std::right << kDefaultInterpolationPeriod << "][ 0 <= i <= p/2 ]" << std::endl;  // NOLINT
  *stream << "       -k    : filtering without gain       (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(!kDefaultGainFlag) << "]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  kfile:" << std::endl;
  *stream << "       filter (PARCOR) coefficients         (double)" << std::endl;  // NOLINT
  *stream << "  infile:" << std::endl;
  *stream << "       filter input                         (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       filter output                        (double)" << std::endl;  // NOLINT
  *stream << "  notice:" << std::endl;
  *stream << "       if i = 0, don't interpolate filter coefficients" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a ltcdf [ @e option ] @e kfile [ @e infile ]
 *
 * - @b -m @e int
 *   - order of coefficients @f$(0 \le M)@f$
 * - @b -p @e int
 *   - frame period @f$(1 \le P)@f$
 * - @b -i @e int
 *   - interpolation period @f$(0 \le I \le P/2)@f$
 * - @b -k @e bool
 *   - filtering without gain
 * - @b kfile @e str
 *   - double-type PARCOR coefficients
 * - @b infile @e str
 *   - double-type input sequence
 * - @b stdout
 *   - double-type output sequence
 *
 * In the below example, an exciation signal generated from pitch information is
 * passed through the synthesis filter built from PARCOR coefficients.
 *
 * @code{.sh}
 *   excite < data.pitch | ltcdf data.rc > data.syn
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_filter_order(kDefaultNumFilterOrder);
  int frame_period(kDefaultFramePeriod);
  int interpolation_period(kDefaultInterpolationPeriod);
  bool gain_flag(kDefaultGainFlag);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "m:p:i:kh", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_filter_order) ||
            num_filter_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("ltcdf", error_message);
          return 1;
        }
        break;
      }
      case 'p': {
        if (!sptk::ConvertStringToInteger(optarg, &frame_period) ||
            frame_period <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -p option must be a positive integer";
          sptk::PrintErrorMessage("ltcdf", error_message);
          return 1;
        }
        break;
      }
      case 'i': {
        if (!sptk::ConvertStringToInteger(optarg, &interpolation_period) ||
            interpolation_period < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -i option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("ltcdf", error_message);
          return 1;
        }
        break;
      }
      case 'k': {
        gain_flag = false;
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

  if (frame_period / 2 < interpolation_period) {
    std::ostringstream error_message;
    error_message << "Interpolation period must be equal to or less than half "
                  << "frame period";
    sptk::PrintErrorMessage("ltcdf", error_message);
    return 1;
  }

  // Get input file names.
  const char* filter_coefficients_file;
  const char* filter_input_file;
  const int num_input_files(argc - optind);
  if (2 == num_input_files) {
    filter_coefficients_file = argv[argc - 2];
    filter_input_file = argv[argc - 1];
  } else if (1 == num_input_files) {
    filter_coefficients_file = argv[argc - 1];
    filter_input_file = NULL;
  } else {
    std::ostringstream error_message;
    error_message << "Just two input files, kfile and infile, are required";
    sptk::PrintErrorMessage("ltcdf", error_message);
    return 1;
  }

  // Open stream for reading filter coefficients.
  std::ifstream ifs1;
  ifs1.open(filter_coefficients_file, std::ios::in | std::ios::binary);
  if (ifs1.fail()) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << filter_coefficients_file;
    sptk::PrintErrorMessage("ltcdf", error_message);
    return 1;
  }
  std::istream& stream_for_filter_coefficients(ifs1);

  // Open stream for reading input signals.
  std::ifstream ifs2;
  ifs2.open(filter_input_file, std::ios::in | std::ios::binary);
  if (ifs2.fail() && NULL != filter_input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << filter_input_file;
    sptk::PrintErrorMessage("ltcdf", error_message);
    return 1;
  }
  std::istream& stream_for_filter_input(ifs2.fail() ? std::cin : ifs2);

  // Prepare variables for filtering.
  const int filter_length(num_filter_order + 1);
  std::vector<double> filter_coefficients(filter_length);
  sptk::InputSourceFromStream input_source(false, filter_length,
                                           &stream_for_filter_coefficients);
  sptk::InputSourceInterpolation interpolation(
      frame_period, interpolation_period, true, &input_source);
  const sptk::InputSourcePreprocessingForFilterGain::FilterGainType gain_type(
      gain_flag
          ? sptk::InputSourcePreprocessingForFilterGain::FilterGainType::kLinear
          : sptk::InputSourcePreprocessingForFilterGain::FilterGainType::
                kUnity);
  sptk::InputSourcePreprocessingForFilterGain preprocessing(gain_type,
                                                            &interpolation);
  if (!preprocessing.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize InputSource";
    sptk::PrintErrorMessage("ltcdf", error_message);
    return 1;
  }

  sptk::AllPoleLatticeDigitalFilter filter(num_filter_order);
  sptk::AllPoleLatticeDigitalFilter::Buffer buffer;
  if (!filter.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize AllPoleLatticeDigitalFilter";
    sptk::PrintErrorMessage("ltcdf", error_message);
    return 1;
  }

  double signal;

  while (sptk::ReadStream(&signal, &stream_for_filter_input)) {
    if (!preprocessing.Get(&filter_coefficients)) {
      std::ostringstream error_message;
      error_message << "Cannot get filter coefficients";
      sptk::PrintErrorMessage("ltcdf", error_message);
      return 1;
    }

    if (!filter.Run(filter_coefficients, &signal, &buffer)) {
      std::ostringstream error_message;
      error_message << "Failed to apply all-pole lattice digital filter";
      sptk::PrintErrorMessage("ltcdf", error_message);
      return 1;
    }

    if (!sptk::WriteStream(signal, &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write a filter output";
      sptk::PrintErrorMessage("ltcdf", error_message);
      return 1;
    }
  }

  return 0;
}
